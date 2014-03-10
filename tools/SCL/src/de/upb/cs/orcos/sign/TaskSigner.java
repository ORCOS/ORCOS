package de.upb.cs.orcos.sign;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
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
	            in.skip(52);
	            int crcstart = hton(readWord(in));            
	            int crcend = hton(readWord(in)); 
	            
	            // TODO: detect the following two positions by parsing the headers
	            int CRCFieldOffset = 68;	            
	            int taskStartOffset = 72;
	            int taskEndOffset = taskStartOffset + (crcend - crcstart);
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
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		if (args.length < 2) return;
		
		if (args[0].equals("crc")) {
			generateCRC(args[1]);
		}
		
		

	}

}

