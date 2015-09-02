package de.upb.cs.orcos.sign;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.CRC32;
import java.util.zip.Checksum;

class Symbol {
    String   name;
    long     address;
    long     length;
    String   section;
}


public class TaskSigner {


    public static boolean IsBigEndian = true;

    public static int hton(int value) {
        if (IsBigEndian) return value;

        int temp = (value >> 24 ) & 0xff;
        temp |=  ((value >> 16 ) & 0xff) << 8;
        temp |=  ((value >> 8 ) & 0xff) << 16;
        temp |=  ((value >> 0 ) & 0xff) << 24;
        return temp;
    }

    public static int readWord(FileInputStream in) throws IOException {

         int word = in.read() << 24;
         word |= (in.read() << 16);
         word |= (in.read() << 8);
         word |= (in.read());

         return word;
    }

    /*
     * Adds the CRC32 value to a tasks CB.
     */
    public static void generateCRC(String file) {

           FileInputStream in = null;
           RandomAccessFile out = null;
           try {
                in = new FileInputStream(file);

                // identify endianess
                // 0x230f7ae9 ORCOS MAGIC WORD

                int magic_word = readWord(in);

                System.out.print("Magic Word: 0x" + Integer.toHexString(magic_word));

                if (magic_word == 0x230f7ae9) {
                    IsBigEndian = true;
                } else if (magic_word == 0xe97a0f23) {
                    IsBigEndian = false;
                } else {
                    System.out.println("Could not identify Endianess.");
                    return;
                }

                if (hton(magic_word) != 0x230f7ae9) {
                    System.out.println("Error in hton()");
                }

                if (IsBigEndian)
                    System.out.println(" => Big Endian Architecture");
                else
                    System.out.println(" => Little Endian Architecture");


                if (hton(readWord(in)) != 1) {
                    System.err.println("Error: Next Header is not CRC! ");
                }

                // move to task start (VMA)
                in.skip(60);
                int crcstart = hton(readWord(in));
                int crcend   = hton(readWord(in));

                // TODO: detect the following two positions by parsing the headers
                int CRCFieldOffset  = 76;
                int taskStartOffset = 80;
                int taskEndOffset   = taskStartOffset + (crcend - crcstart);
                System.out.println("Calculating checksum over " + Integer.toHexString(crcstart) + " - " + Integer.toHexString(crcend));
                System.out.println("File position: " + taskStartOffset + " - " + taskEndOffset);

                // sanity checks
                if (taskEndOffset < taskStartOffset) return;
                if (taskStartOffset < 0) return;
                if (taskEndOffset < 0) return;

                in.close();
                in = new FileInputStream(file);

                in.skip(taskStartOffset);
                Checksum crc = new CRC32();
                crc.reset();
                int pos = taskStartOffset;
                while (pos < taskEndOffset) {
                    int b = in.read();
                    crc.update(b);
                    pos++;
                }

                int crcvalue = (int) crc.getValue();

                System.out.println("CRC32: 0x" + Integer.toHexString(crcvalue));
                crcvalue = hton(crcvalue);

                in.close();
                out = new RandomAccessFile(file,"rw");
                out.seek(CRCFieldOffset);
                out.writeInt(crcvalue);
                out.close();

            } catch(Exception e) {
                System.out.println(e.toString());
            }

    }


      public static void generateStringTable(String file) throws IOException {
            BufferedReader br = new BufferedReader(new FileReader(file));

            HashMap<Long,Symbol> symbols = new HashMap<Long,Symbol>();

            /* first parse the map file */
            Pattern section           = Pattern.compile("\\s*\\.([a-zA-Z_]+).*");
            Pattern sectionlen        = Pattern.compile("\\s*\\.([a-zA-Z_]+)\\s+0x([0-9a-f]+)\\s+0x([0-9a-f]+).*");
            Pattern regex_symbol      = Pattern.compile("\\s+0x([0-9a-f]+)\\s+(.+)");
            try {
              String line = br.readLine();
              String currentSection = null;
              long sectionAddr   = 0;
              long sectionLength = 0;
              long lastSymbolAddr = 0;
              Symbol lastSymbol = null;
              while (line != null) {
                   /* read the complete file*/

                  Matcher sectionMatcher = sectionlen.matcher(line);
                  if(sectionMatcher.matches()) {
                      currentSection = sectionMatcher.group(1);
                      sectionAddr    = Long.parseLong(sectionMatcher.group(2),16);
                      sectionLength  = Long.parseLong(sectionMatcher.group(3),16);
                  }

                  /* check for a symbol */
                  Matcher symbolMatcher = regex_symbol.matcher(line);
                  if (symbolMatcher.matches() && sectionLength > 0) {
                      /* found a symbol definition like 0xaddress  Symbol*/
                      long address = Long.parseLong(symbolMatcher.group(1),16);

                      if (lastSymbol != null) {
                          if (lastSymbol.length == 0) {
                              /* update the length as it is now known */
                              lastSymbol.length = address - lastSymbol.address;
                              if (lastSymbol.length < 0) {
                                  lastSymbol.length = 0;
                              }
                          }
                      }

                      String symbolname = symbolMatcher.group(2);
                      Symbol s  = new Symbol();
                      s.address = address;
                      s.name = symbolname;
                      if (lastSymbolAddr != 0) {
                          s.length = address - lastSymbolAddr;
                      }
                      s.section = currentSection;
                      symbols.put(new Long(address), s);
                      lastSymbol = s;
                  }

                  line = br.readLine();
              }

            } catch (Exception e) {
                e.printStackTrace();
            }



            try {
                int entries = 0;
                String outfile = file;
                outfile = outfile.substring(0, outfile.lastIndexOf("."));
                outfile += "_strtable.c";
                System.out.println("Outfile: " + outfile);
                BufferedWriter writer = new BufferedWriter(new FileWriter(outfile));
                writer.write("typedef struct { unsigned int address;  unsigned int length; char signature[64];} __attribute__((packed)) DebugStrTableEntriesT;\ntypedef struct { unsigned int numEntries; DebugStrTableEntriesT strTable[]; } __attribute__((packed)) DebugStrTableT;\nDebugStrTableT debug_strtable = {");

                String strentries = "";

                SortedSet<Long> keys = new TreeSet<Long>(symbols.keySet());
                for (Long key : keys) {
                    Symbol s = symbols.get(key);

                    if (s.section.startsWith("text")) {
                        // write entry
                        //System.out.println("Address: " + s.address + " length: " + (s.length) + " Method: " + s.name);
                        String signature = s.name.replace("\\", "_");
                        signature.trim();
                        signature = signature.substring(0, Math.min(63, signature.length()));

                        strentries += "{ 0x" + Long.toHexString(s.address) + ", " + (s.length) + ", \"" + signature + "\"},\n";
                        entries++;
                    }
                }
                writer.write("" + entries +",{\n");
                writer.write(strentries);
                writer.write("}};");
                System.out.println("Debug Table contains " + entries + " Symbols\n");
                writer.close();
            } catch (Exception e) {
              e.printStackTrace();
            }
            br.close();

        }

    /**
     * @param args
     * @throws IOException
     */
    public static void main(String[] args) throws IOException {

        if (args.length < 2) return;

        if (args[0].equals("crc")) {
            generateCRC(args[1]);
        }

        if (args[0].equals("stringtable")) {
            System.out.println("Generating string table");
            try {
            generateStringTable(args[1]);
            } catch (Exception e) {
                System.out.println(e);
            }
        }

    }

}

