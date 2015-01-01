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
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.CRC32;
import java.util.zip.Checksum;

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
                int CRCFieldOffset  = 68;
                int taskStartOffset = 72;
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

        try {

            String outfile = file;
            outfile = outfile.substring(0, outfile.lastIndexOf("."));
            outfile += "_strtable.c";
            System.out.println("Outfile: " + outfile);
            BufferedWriter writer = new BufferedWriter(new FileWriter(outfile));
            writer.write("typedef struct {  unsigned int address;  unsigned int length; char* signature;} DebugStrTableT;\n DebugStrTableT debug_strtable[] = {");
            String line = br.readLine();

            long address = -1;
            int length = 0;
            String signature;
            boolean textSection = false;

            Pattern regex_methodrange = Pattern.compile("\\s+0x([0-9a-f]+)\\s+0x([0-9a-f]+).*");
            Pattern regex_methodname  = Pattern.compile("\\s+0x([0-9a-f]+)\\s+(.+).*");
            Pattern textsection       = Pattern.compile("\\s*\\.text\\.(.+)");
            Pattern section           = Pattern.compile("\\s*\\.([a-zA-Z_]+)\\..+");

            Pattern sectionlen        = Pattern.compile("\\s*\\.text\\.(.+)\\s+0x([0-9a-f]+)\\s+0x([0-9a-f]+).*");
            int entries = 0;

            while (line != null) {

                signature = "";
                address = 0;
                length = 0;

                // try to detect c-style strings as .text.tcp_tmr  0x8100af80       0x24 ./output/\liborcoskernel.a(tcp.o)
                Matcher mall = sectionlen.matcher(line);
                if(mall.matches()) {
                     signature      = mall.group(1);
                     address        = Long.parseLong(mall.group(2),16);
                     length         = Integer.parseInt(mall.group(3),16);
                     // check if next line contains more readable name
                     line = br.readLine();
                     Matcher nameMatcher = regex_methodname.matcher(line);
                     if (nameMatcher.matches()) {
                         signature = nameMatcher.group(2);
                     }
                     writer.write("{ 0x" + Long.toHexString(address) + ", " + length + ", \"" + signature + "\"},\n");
                    entries++;
                }

                Matcher m0 = section.matcher(line);
                if (m0.matches()) {
                    if (m0.group(1).equals("text")) {
                        textSection    = true;
                        address        = 0;
                        length         = 0;

                        Matcher m1     = textsection.matcher(line);
                        if (m1.matches()) {
                            signature      = m1.group(1);
                           // System.out.println(" Method: " + signature);
                        }


                        line = br.readLine();
                        Matcher m = regex_methodrange.matcher(line);
                        if (m.matches()) {
                            // method start
                            address = Long.parseLong(m.group(1),16);
                            length  = Integer.parseInt(m.group(2),16);

                            line = br.readLine();

                            Matcher m2 = regex_methodname.matcher(line);
                            if (m2.matches()) {
                                signature = m2.group(2);
                                if (address != 0 && length > 0 && textSection) {
                                    //System.out.println("Address: " + address + " length: " + length + " Method: " + signature);
                                    writer.write("{ 0x" + Long.toHexString(address) + ", " + length + ", \"" + signature + "\"},\n");
                                    entries++;
                                }
                            } else {
                                // just add using the section name
                                if (address != 0 && length > 0 && textSection && !signature.equals("")) {
                                    //System.out.println("Address: " + address + " length: " + length + " Method: " + signature);
                                    writer.write("{ 0x" + Long.toHexString(address) + ", " + length + ", \"" + signature + "\"},\n");
                                    entries++;
                                }
                            }

                        }

                    }
                }


                line = br.readLine();
            }

            writer.write("};\n unsigned int debug_strtable_entries = " + entries +";\n");
            writer.close();
        } finally {
          br.close();
        }


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

