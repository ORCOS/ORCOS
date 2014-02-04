/**  
	ORCOS - an Organic Reconfigurable Operating System
	Copyright (C) 2008 University of Paderborn
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package de.upb.cs.orcos.xml;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

import javax.xml.XMLConstants;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.jdom.Attribute;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.xml.sax.SAXException;

/**
 * This class can be used to convert an xml-scl-definition into appropriate
 * orcos-c-headers. Furthermore it constructs needed files for task generation.
 * For more information see the SCL part of doxygen documentation.
 * 
 * 
 * @author Matthias Grawinkel (grawinkel@upb.de)
 */
public class SCLReader {

	private static final String DEFAULT_TARGET_SCL_CONFIG_NAME = "make/SCLConfig.hh";
	private static final String DEFAULT_TARGET_TASK_CONFIG_NAME = "make/tasktable.S";
	private static final String DEFAULT_TARGET_LOGGING_CONFIG_NAME = "make/logger_config.hh";
	
	private static final String DEFAULT_TARGET_MAKEFILE_NAME = "make/scl_make.mk";

	private static final String TASK_SED_FILENAME = "task.sed";
	//private static final String TASK_MAKE_FILENAME = "tasks.sh";
	//private static final String TASK_CLEAN_FILENAME = "tasks_clean.sh";
	private static String fileSeparator = System.getProperty("file.separator");
	private static File outputDir;

	private static ArrayList<String> arch_objects = new ArrayList<String>();
	private static ArrayList<String> module_objects = new ArrayList<String>();
	private static ArrayList<String> module_addresses = new ArrayList<String>();
	
	private static StringBuffer archdirbuffer = new StringBuffer();
	private static StringBuffer tasksbuffer = new StringBuffer();
	private static StringBuffer tasksdirbuffer = new StringBuffer();
	private static StringBuffer tasksstartbuffer = new StringBuffer();
	private static StringBuffer tasksendbuffer = new StringBuffer();
	
	
	private static ArrayList<String> typedefs = new ArrayList();
	
	// incremented for each task that is found in the config file. this number
	// is written as first entry to the DEFAULT_TARGET_TASK_CONFIG_NAME
	private static int numberOfTasks = 0;

	private static int verboselevel = 0;
	
	/**
	 * Main method which is called on program startup.
	 * @param args The command line arguments passed to the program
	 */
	public static void main(String[] args) {

		if (args.length < 2) {
			System.out
					.println("Usage: path-to-xml-file path-to-xml-schema-file [--output-dir path-to-output-dir][--check-deps path-to-deps-xml-file path-to-deps-schema-file]");
			System.exit(1);
		}

		String xmlInputFileName = args[0];
		String xmlSchemaFileName = args[1];

		String dependencyXML = "";
		String dependencySchema = "";

		// check whether the configfile should be validated against the deps-xml
		boolean checkAgainstDeps = false;

		for (int i = 1; i < args.length; i++) {
			if (args[i].equals("--check-deps")) {
				dependencyXML = args[++i];
				dependencySchema = args[++i];
				try {
					if (verboselevel > 0)
					System.out.println("Checking against dependencies file: "
							+ new File(dependencyXML).getAbsolutePath());
					
					if (validate(dependencySchema, dependencyXML)) {
						
						if (verboselevel > 0)
						System.out
								.println("SCL Dependencies Definition is valid.");
						checkAgainstDeps = true;
						
					}
				} catch (Exception e) {
					System.err
							.println("Error while validating SCL Dependencies Definition "
									+ dependencySchema);
					System.err.println(e.getMessage());
					System.exit(1);
				}
			} else if (args[i].equals("--output-dir")) {
				outputDir = new File(args[++i]);
				if (!outputDir.canWrite()) {
					System.err
							.println("Target output-dir is either illegal or not writable");
				}
			} else if (args[i].equals("--verbose=1")) {
				verboselevel = 1;
			} else if (args[i].equals("--verbose=2")) {
				verboselevel = 2;
			} else if (args[i].equals("--verbose=3")) {
				verboselevel = 3;
			}
		}

		File xmlInputFile = new File(xmlInputFileName);
		if (outputDir == null) {
			// no target dir given, use dir of config file as output dir
			outputDir = new File(System.getProperty("user.dir"));
		}
		
		if (verboselevel > 0)
		System.out.println("Transforming file: "
				+ xmlInputFile.getAbsolutePath());

		try {
			if (validate(xmlSchemaFileName, xmlInputFileName)) {
				if (verboselevel > 1)
				System.out.println("SCL configuration is valid!");
			}
		} catch (Exception e) {
			System.err.println("Error while validating xml config file");
			System.err.println(e.getMessage());
			System.exit(1);
		}

		// This is written to SCLConfig.hh
		StringBuffer sclConfigBuffer = new StringBuffer();

		sclConfigBuffer.append("#ifndef _SCLCONFIG_HH_\r\n#define _SCLCONFIG_HH_\r\n");
		
		// This is written to tasktable.S
		StringBuffer taskTableBuffer = new StringBuffer();

		 tasksdirbuffer.append("TASKS=(");
		 tasksstartbuffer.append("TASKS_START=(");
		 tasksendbuffer.append("TASK_END=(");

		try {
			// check against deps-xml
			transformXML(checkAgainstDeps, xmlInputFileName, dependencyXML,
					sclConfigBuffer, taskTableBuffer);

		} catch (Exception e) {
			System.err.println("Error while transforming");
			System.err.println(e.getMessage());
			System.exit(1);
		}

		sclConfigBuffer.append("#endif");
		
		
		tasksdirbuffer.append(")\n");
		tasksstartbuffer.append(")\n");
		tasksendbuffer.append(")\n");
		
		// now write the config files

		try {
			// write sclconfig to header file

			FileWriter output = new FileWriter(outputDir.getAbsolutePath()
					+ fileSeparator + DEFAULT_TARGET_SCL_CONFIG_NAME);
			BufferedWriter bufWrite = new BufferedWriter(output);
			bufWrite.write(sclConfigBuffer.toString());
			bufWrite.flush();
			bufWrite.close();
			output.close();
			
			output = new FileWriter(outputDir.getAbsolutePath()
					+ fileSeparator + "/make/tasks.in");
			bufWrite = new BufferedWriter(output);
			bufWrite.write(tasksdirbuffer.toString());
			bufWrite.write(tasksstartbuffer.toString());
			bufWrite.write(tasksendbuffer.toString());
			bufWrite.flush();
			bufWrite.close();
			output.close();

			output = new FileWriter(outputDir.getAbsolutePath()
					+ fileSeparator + DEFAULT_TARGET_MAKEFILE_NAME);
			bufWrite = new BufferedWriter(output);			
			for (String s : arch_objects)
				bufWrite.write("ARCH_OBJ +=" + s + "\n");

			for (String s : module_objects)
				bufWrite.write("MODULE_OBJ +=" + s + "\n");

			for (String s : module_addresses)
				bufWrite.write(s + "\n");
						
			bufWrite.write("TASKS = ");
			bufWrite.write(tasksbuffer.toString());								
			bufWrite.write("\n");
			
			bufWrite.write(archdirbuffer.toString());
			
			bufWrite.flush();
			bufWrite.close();
			output.close();
			
			// write tasks to linker file
			output = new FileWriter(outputDir.getAbsolutePath() + fileSeparator
					+ DEFAULT_TARGET_TASK_CONFIG_NAME);
			bufWrite = new BufferedWriter(output);
			bufWrite.write(".section .tasktable\n");
			bufWrite.write(".long " + numberOfTasks + ";\n");
			bufWrite.write(taskTableBuffer.toString());
			bufWrite.flush();
			bufWrite.close();
			output.close();

		} catch (Exception e) {
			System.err.println("Error while writing files");
			System.err.println(e.getClass());
			System.err.println(e.getMessage());
			System.exit(1);
		}

		if (verboselevel > 0)
		System.out.println("Files written to: " + outputDir.getAbsolutePath());
		System.exit(0);
	}

	/**
	 * validate the given xmlfile against the given xsd file.
	 * 
	 * @param xsdfile
	 *            *.xsd
	 * @param xmlfile
	 *            *.xml
	 * @return true if the file is ok, throws an exception otherwise.
	 * @throws SAXException
	 * @throws IOException
	 * @throws Exception
	 */
	private static boolean validate(String xsdfile, String xmlfile)
			throws SAXException, IOException {

		final String sl = XMLConstants.W3C_XML_SCHEMA_NS_URI;
		SchemaFactory factory = SchemaFactory.newInstance(sl);
		
		if (verboselevel > 1)
		System.out.println("Using XSD file: "
				+ new File(xsdfile).getAbsolutePath());
		
		StreamSource streamSource = new StreamSource(xsdfile);
		Schema schema = factory.newSchema(streamSource);
		Validator validator = schema.newValidator();		
		validator.validate(new StreamSource(xmlfile));

		return true;
	}

	/**
	 * Transforms the given xml file.
	 * 
	 */
	@SuppressWarnings("unchecked")
	private static void transformXML(boolean checkDeps, String filename,
			String depsFile, StringBuffer configBuffer,
			StringBuffer taskTableBuffer) throws Exception {

		SAXBuilder saxBuilder = new SAXBuilder(false);

		Document document = saxBuilder.build(new File(filename));

		List<Element> rootChildList = document.getRootElement().getChildren();

		// these variables are only created and later used if --check-deps
		// parameter was supplied
		Document dependenciesDocument;
		List<Element> dependenciesList = null;

		if (checkDeps) {
			dependenciesDocument = saxBuilder.build(new File(depsFile));
			dependenciesList = dependenciesDocument.getRootElement().getChildren();
		}

		// iterate over all found nodes and process them
		for (Element childElement : rootChildList) {
			// Process Options Node
			if (childElement.getName().equalsIgnoreCase("Options")) {
				processConfigNode(childElement, configBuffer);

			} else if (childElement.getName().equalsIgnoreCase("Tasks")) {
				// Process Task Node
				List<Element> taskNodes = childElement.getChildren();

				for (Element taskNode : taskNodes) {
					processTaskNode(taskNode, taskTableBuffer);
				}

			} else if (childElement.getName().equalsIgnoreCase("SCL")) {
				// Process SCL Node
				List<Element> sclNodes = childElement.getChildren();

				for (Element sclNode : sclNodes) {
					if (sclNode.getName().equalsIgnoreCase("Skeleton")) {

						if (!checkDeps) {
							processSkeletonNode(sclNode, configBuffer);
						} else {
							// process each SKELETON
							String skeletonName = sclNode.getChildText("Name").trim();

							boolean found = false;
							// find corresponding node in dependencies File
							for (Element dependencyElement : dependenciesList) {
								if (dependencyElement.getChildText("Name").equals(skeletonName)) {

									if (checkSkeletonDependencies(sclNode,dependencyElement)) {
										// the current node is valid against its
										// dependencies
										processSkeletonNode(sclNode,configBuffer);
										found = true;
										break;

									} else {

										throw new Exception("Skeleton Node: "+ childElement.getChildText("Name")
														+ " does not match its dependencies");
									}
								}
							}

							// check if the corresponding node was found in
							// dependencies file
							if (!found) {
								throw new Exception("Skeleton Node: "
												+ skeletonName
												+ " - no corresponding dependencies could be found in SCL Dependencies file!");
							}
						}

					} else if (sclNode.getName().equalsIgnoreCase("Define")) {
						processDefineNode(sclNode, configBuffer);
					}
				}
			}
		}
	}

	@SuppressWarnings("unchecked")
	private static void processConfigNode(Element node,
			StringBuffer configBuffer) throws Exception {

		Element loggingNode = node.getChild("Logging");
		if (loggingNode == null) {
			return;
		}

		StringBuffer loggerConfigBuffer = new StringBuffer();

		loggerConfigBuffer.append("#ifndef LOGGERCONFIG_HH_\n");
		loggerConfigBuffer.append("#define LOGGERCONFIG_HH_\n\n");

		loggerConfigBuffer.append("\n\n //Global logger config\n\n");

		loggerConfigBuffer
				.append("enum Level {FATAL=0,ERROR=1,WARN=2,INFO=3,DEBUG=4,TRACE=5};\n\n");

		loggerConfigBuffer.append("enum Prefix {\n");

		boolean first = true;

		List<Element> logNodes = loggingNode.getChildren();

		for (Element logNode : logNodes) {

			String prefix = logNode.getAttribute("Prefix").getValue();
			String level = logNode.getAttribute("Level").getValue();

			if (first) {
				loggerConfigBuffer.append(prefix + "=" + level);
				first = false;
			} else {
				loggerConfigBuffer.append(",\n" + prefix + "=" + level);
			}
		}
		loggerConfigBuffer.append("\n};\n");

		loggerConfigBuffer.append("#endif\n\n");

		FileWriter output = new FileWriter(outputDir.getAbsolutePath()
				+ fileSeparator + DEFAULT_TARGET_LOGGING_CONFIG_NAME);
		BufferedWriter bufWrite = new BufferedWriter(output);
		bufWrite.write(loggerConfigBuffer.toString());
		bufWrite.flush();
		bufWrite.close();
		output.close();
	}

	@SuppressWarnings("unchecked")
	private static boolean checkSkeletonDependencies(Element skeleton,
			Element dependencies) throws Exception {

		// Check if a superclass used in SCLConfig skeleton is defined in
		// SCLDependencies
		String skeletonName = skeleton.getChildText("Name");
		Element superclassElement = skeleton.getChild("Superclass");
		if (superclassElement != null) {

			// superclass exists in skeleton
			String superclassValue = superclassElement.getValue();
			if (verboselevel > 2)
			System.out.println("Checking Superclass: " + superclassValue);

			// search for appropriate superclass definition in SCLDependencies
			Element depSuperclasses = dependencies.getChild("Superclasses");
			if (depSuperclasses == null) {
				System.err.println("No Superclasses in dependency file found..");
				return false;
			}
			
			boolean found = false;
			List<Element> depSuperclassList = depSuperclasses.getChildren();
			for (Element depSuperclassItem : depSuperclassList) {

				// System.out.println("Diffing: " + dClass.getValue() +
				// " against: " + superclass);
				if (depSuperclassItem.getAttributeValue("Name").equals(superclassValue)) {
					found = true;
					break;
				}
			}
			if (!found) {
				throw new Exception("Skeleton: " + skeletonName
						+ " had no valid superclass: " + superclassValue);
			}
		}
		// the superclass, if defined, was correct

		// Check if the member class used in SCLConfig skeleton is defined in
		// SCLDependencies
		List<Element> memberList = skeleton.getChildren("Member");
		List<Element> depMemberList = dependencies.getChildren("Member");

		for (Element memberItem : memberList) {

			String memberName = memberItem.getAttributeValue("Name").trim();
			String memberClass = memberItem.getAttributeValue("Class","").trim();
			// ignore members which do not have a configurable class
			if (memberClass == "") continue;
			
			boolean found = false;

			for (Element depMemberItem : depMemberList) {

				String depMemberName = depMemberItem.getChildText("Name");
				if (memberName.equals(depMemberName)) {

					List<Element> definedClasses = depMemberItem.getChild("Classes").getChildren();
					for (Element depClassItem : definedClasses) {

						String depClassValue = depClassItem.getAttributeValue("Name").trim();
						if (memberClass.equals(depClassValue)) {
							found = true;
							if (verboselevel > 1)
							System.out.println("Checked member: " + memberName
									+ " ::= " + memberClass);
							break;
						}
					}
				}
			}
			if (!found) {
				throw new Exception("Member: " + memberName
						+ " had no valid Class definition: " + memberClass);
			}
		}
		return true;
	}

	/**
	 * Appends a #define to the outbuffer
	 * 
	 * @param defineNode
	 * @param outBuffer
	 */
	private static void processDefineNode(Element defineNode,
			StringBuffer outBuffer) {
		
		outBuffer.append("\r\n#define " + defineNode.getChildText("Name") + " "
				+ defineNode.getChildText("Value") + "\r\n");
		
		if (defineNode.getChildText("Name").equals("ENABLE_NETWORKING") && defineNode.getChildText("Value").equals("1"))
		{
			// add networking objects
			arch_objects.add("Socket.o");
			arch_objects.add("ProtocolPool.o");
			arch_objects.add("TCPTransportProtocol.o");
			arch_objects.add("IPv4AddressProtocol.o");
			arch_objects.add("etharp.o");
			arch_objects.add("ip.o");
			arch_objects.add("init.o");
			arch_objects.add("mem.o");
			arch_objects.add("memp.o");
			arch_objects.add("pbuf.o");
			arch_objects.add("netif.o");
			arch_objects.add("udp.o");
			arch_objects.add("tcp.o");
			arch_objects.add("sys.o");
			arch_objects.add("inet.o");
			arch_objects.add("ethernet.o");
			arch_objects.add("ip4_addr.o");
			arch_objects.add("ip4.o");
			arch_objects.add("ip4_frag.o");
			arch_objects.add("tcp_in.o");
			arch_objects.add("tcp_out.o");
			arch_objects.add("stats.o");
			arch_objects.add("icmp.o");
			arch_objects.add("ip6_addr.o");
			arch_objects.add("ip6.o");
			arch_objects.add("dhcp.o");
			arch_objects.add("icmp.o");
			arch_objects.add("icmp6.o");
			arch_objects.add("ethar.o");
			arch_objects.add("ethndp.o");
			arch_objects.add("lwipTMR.o");		
			arch_objects.add("CAB.o");		
		}
		
		// add workertask files
		if (defineNode.getChildText("Name").equals("USE_WORKERTASK") && defineNode.getChildText("Value").equals("1"))
		{
			arch_objects.add("WorkerThread.o");
			arch_objects.add("WorkerTask.o");		
		}
		if (defineNode.getChildText("Name").equals("HAS_PROCFS_ENABLED") && defineNode.getChildText("Value").equals("1"))
		{
			arch_objects.add("SimpleDebugCollector.o");
			arch_objects.add("SimpleProcfs.o");	
		}
		
	}

	/**
	 * Appends a #define to the outbuffer
	 * 
	 * @param defineNode
	 * @param outBuffer
	 */
	private static void processTaskNode(Element taskNode,
			StringBuffer taskBuffer) {

		// TODO: autoadd ending / to path, check addr etc for nullpointers

		String start = taskNode.getChildText("Start");
		if (start != null) {
			taskBuffer.append(".long " + start + ";\n");
		}
		
		tasksbuffer.append(taskNode.getChildText("Path") + " ");
		tasksdirbuffer.append(taskNode.getChildText("Path") + " ");
		tasksstartbuffer.append(taskNode.getChildText("Start") + " ");
		tasksendbuffer.append(taskNode.getChildText("End") + " ");
	
		numberOfTasks++;

		try {
			String sedPath = taskNode.getChildText("Path") + TASK_SED_FILENAME;
			FileWriter fileWriter = new FileWriter(sedPath);

			if (start != null) {
				fileWriter.write("s/TASK_START/" + start + "/\n");
			}

			String heap = taskNode.getChildText("Heap");
			if (heap != null) {
				fileWriter.write("s/TASK_HEAP/" + heap + "/\n");
				taskBuffer.append(".long " + heap + ";\n");
			}

			String vma = taskNode.getChildText("Vma");
			if (vma == null) {
				fileWriter.write("s/TASK_VMA/" + start + "/\n");
			} else {
				fileWriter.write("s/TASK_VMA/" + vma + "/\n");
			}

			String end = taskNode.getChildText("End");
			if (end != null) {
				fileWriter.write("s/TASK_END/" + end + "/\n");
				taskBuffer.append(".long " + end + ";\n");
			}

			// get PriorityOptions

			Element priorityOptions = taskNode.getChild("PriorityOptions");

			if (priorityOptions != null) {

				String initialPriority = priorityOptions
						.getChildText("InitialPriority");
				if (initialPriority != null) {
					fileWriter.write("s/INITIAL_PRIORITY/" + initialPriority
							+ "/\n");
				} else {
					fileWriter.write("s/INITIAL_PRIORITY/" + 0 + "/\n");
				}

				String phase = priorityOptions.getChildText("Phase");
				if (phase != null) {
					fileWriter.write("s/PHASE/" + phase + "/\n");
				} else {
					fileWriter.write("s/PHASE/" + 0 + "/\n");
				}

				// get <RealTimeOptions>
				Element realTimeOptions = priorityOptions
						.getChild("RealTimeOptions");

				if (realTimeOptions != null) {
					String period = realTimeOptions.getChildText("Period");

					if (period != null) {
						fileWriter.write("s/PERIOD/" + period + "/\n");
					} else {
						fileWriter.write("s/PERIOD/" + 0 + "/\n");
					}

					String deadline = realTimeOptions.getChildText("Deadline");
					if (deadline != null) {
						fileWriter.write("s/DEADLINE/" + deadline + "/\n");
					} else {
						fileWriter.write("s/DEADLINE/" + 0 + "/\n");
					}

					String executionTime = realTimeOptions
							.getChildText("ExecutionTime");
					if (executionTime != null) {
						fileWriter.write("s/EXECUTIONTIME/" + executionTime
								+ "/\n");
					} else {
						fileWriter.write("s/EXECUTIONTIME/" + 0 + "/\n");
					}
				}
			} else {
				// write default values for period, deadline, exectime
				fileWriter.write("s/INITIAL_PRIORITY/" + 0 + "/\n");
				fileWriter.write("s/PHASE/" + 0 + "/\n");

				fileWriter.write("s/PERIOD/" + 0 + "/\n");
				fileWriter.write("s/DEADLINE/" + 0 + "/\n");
				fileWriter.write("s/EXECUTIONTIME/" + 0 + "/\n");
			}

			fileWriter.close();

		} catch (IOException ex) {
			System.out.println("Could not write "
					+ taskNode.getChildText("Path") + TASK_SED_FILENAME);
			System.exit(1);
		}
	}

	/**
	 * Appends a SKELETON node to the outbuffer
	 * 
	 * @param skeletonNode
	 * @param outBuffer
	 * @throws Exception 
	 */
	@SuppressWarnings("unchecked")
	private static void processSkeletonNode(Element skeletonNode,
			StringBuffer outBuffer) throws Exception {

		String templateString = "";
		// required
		String className = skeletonNode.getChild("Name").getValue().trim();		
		outBuffer.append("\r\n// Configuration of class " + className + "\r\n #ifndef __ASSEMBLER__\r\n");

		// optional
		String superClassName = null;

		// ================= <Superclass> ============================

		Element superClassNode = skeletonNode.getChild("Superclass");
		if (superClassNode != null) {
			superClassName = superClassNode.getValue().trim();

			outBuffer.append("#define " + className + "Cfd_hh <"
					+ superClassName + ".hh>\r\n");
			outBuffer.append("#define "	+ className	+ "CfdCl "
					+ superClassName.substring(superClassName.lastIndexOf("/") + 1)
					+ templateString + "\r\n");
			
			if (verboselevel > 2) System.out.println("Superclass #define created for: " + superClassName);
			
		}		
		// ================= </Superclass> ============================

		
		if (className.equals("Board")) {
			// check superclass and set ARCH_DIR appropriately
			archdirbuffer.append("ARCH_DIR = $(KERNEL_DIR)");
			String s = superClassName.substring(0,superClassName.lastIndexOf("/"));
			archdirbuffer.append(s);
			archdirbuffer.append("/\n");
			if (verboselevel > 2) System.out.println("ARCH_DIR: " + s);

		}
		
		List<Element> memberNodes = skeletonNode.getChildren("Member");
		for (Element memberNode : memberNodes) {

			String memberName = memberNode.getAttributeValue("Name").trim();
			String memberClassName = memberNode.getAttributeValue("Class","").trim();

			
			if (verboselevel > 2) System.out.println("Processing '" + memberName + "' (" + memberClassName + ")");

			
			boolean isNone = false;
			boolean isUserspace = false;

			isNone =  memberNode.getAttributeValue("None","true").equalsIgnoreCase("true");
			String phyloadAddr = "";
			String memberClassFileName = memberClassName.substring(memberClassName.lastIndexOf("/")+1);
			
			// only generate module and arch information for the class skeleton
			if (className.equals("Board") && !isNone) {
			
				//System.out.println(memberClassFileName);
				Attribute userSpaceNode = memberNode.getAttribute("UserSpace");
				if (userSpaceNode != null && userSpaceNode.getValue().trim().equalsIgnoreCase("true")) {
					// is inside userspace
					module_objects.add(memberClassFileName + ".o");					
					Element phyLoadAddrNode = memberNode.getChild("PhyLoadAddress");
					if (phyLoadAddrNode == null) {
						System.out.println("No Physical Load Address given for module: " + memberClassFileName);
						throw new Exception();
					}
					
					phyloadAddr = phyLoadAddrNode.getValue();
					module_addresses.add(memberClassFileName.toUpperCase() + "_PHY_LOAD_ADDRESS=" + phyloadAddr);

					FileWriter output = new FileWriter(outputDir.getAbsolutePath()
							+ fileSeparator + "modules/" + memberClassFileName + ".o.sed");
					BufferedWriter bufWrite = new BufferedWriter(output);
					bufWrite.write("s/MODULE_PHY_MEM_ADDR/"+ phyloadAddr +"/g");
					bufWrite.flush();
					bufWrite.close();
					output.close();
					isUserspace = true;
				} else {
					// not in userspace
					if (!memberClassFileName.contains("Dummy")) {
						// only build object if its not a dummy object
						arch_objects.add(memberClassFileName + ".o");
					} else {
						System.out.println("Warning: not adding " + memberClassFileName + " to object list (Dummy* Filter)!");
					}
					
				}
			}
			
			if (!isNone && !className.equals("SyscallManager")){
				// this class is configured so we must compile it
				if (!memberClassFileName.contains("Dummy")) {
					arch_objects.add(memberClassFileName + ".o");	
				}
			}
			
			
			if (superClassName.contains("usb/")){			
				if (!isNone ){					
					arch_objects.add("USBDriverLibrary.o");	
				}
			}
			
			
			if (className.equals("Kernel") && !isNone) {
								
				if (memberName.equals("Thread")) {
					if (memberClassFileName.contains("RealTimeThread"))
						arch_objects.add("PriorityThread.o");
					
				}
				
			}
			
			//Element deviceNode = memberNode.getChild("Device");
			//Element addressNode = memberNode.getChild("Address");
			//Element lengthNode = memberNode.getChild("Length");
			//Element uniqueIDNode = memberNode.getChild("UniqueID");
			//Element ip4addrNode = memberNode.getChild("IP4Addr");
			//Element ip4netmaskNode = memberNode.getChild("IP4NetMask");
			
			// 
			
			outBuffer.append("\r\n// Configuration of member " + memberName
					+ " of class " + className + "\r\n");

			String property_init = "";
			String constructor = ""; //generateConstructorString(memberNode);

			List<Element> properties = memberNode.getChildren("PropertyValue");
			
			if (properties.size() > 0) {
				String inittypename = "T_" + memberClassFileName +"_Init";
				if (! typedefs.contains(inittypename)) {
					typedefs.add(inittypename);
					
					outBuffer.append("\r\ntypedef struct " + inittypename + " { \r\n");
							
					for (Element p : properties) {
						// throw exception if no type given!
						String type = p.getAttributeValue("Type");											
						outBuffer.append("   " + type + " " + p.getAttributeValue("Name") + ";\r\n");					
					}		
				
					outBuffer.append("} " + inittypename + ";\r\n\r\n");
				}
				
				property_init = inittypename + " init_" + memberNode.getAttributeValue("Name") + " = {";
				boolean first = true;
				for (Element p : properties) {
					
					String type = p.getAttributeValue("Type");
					String value;
					if (type.contains("char*")) 
						value = "\"" + p.getAttributeValue("Value") + "\"";
					else value = p.getAttributeValue("Value");
					
					if (!first) property_init += ",";
					else first = false;
					
					property_init += " " + p.getAttributeValue("Name") + " : " + value; 
				}
				property_init += "};";
				constructor = "&init_" + memberNode.getAttributeValue("Name");
			}
			
			String configuredType;
			
			if (!isNone) {
				outBuffer.append("#define HAS_" + className + "_" + memberName
						+ "Cfd 1\r\n");

				outBuffer.append("#define " + className + "_" + memberName
						+ "_IN_USERSPACE " + (isUserspace ? "1" : "0") + "\r\n");
			}
			
			if (memberClassName != "") {
				if (!isUserspace) {
					configuredType = memberClassName.substring(memberClassName
							.lastIndexOf("/") + 1);
					outBuffer.append("#define " + className + "_" + memberName
							+ "_hh <" + memberClassName + ".hh>\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "_cc <" + memberClassName + ".cc>\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "CfdCl " + configuredType	+ "\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "CfdT " + configuredType + "*\r\n");
				} else {
					configuredType = "USCommDeviceDriver";
					
					outBuffer.append("#define " + className + "_" + memberName
							+ "_hh <hal/USCommDeviceDriver.hh>\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "_cc <hal/USCommDeviceDriver.cc>\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "CfdCl " + configuredType	 + "\r\n");
					outBuffer.append("#define " + className + "_" + memberName
							+ "CfdT " + configuredType + "*\r\n");
				}
				
				
				
				// Generate Member definition 			
				outBuffer.append("#define DEF_" + className + "_" + memberName
						+ "Cfd \\\r\n");
	
				if (!isNone) {
					outBuffer.append("private: \\\r\n");
					outBuffer.append("    " + configuredType+  "* "	+ memberName + "Cfd; \\\r\n");
				}
	
				outBuffer.append("public: \\\r\n");
	
				if (!isNone) {
	
					outBuffer.append("    void set" + memberName + "("
							+ configuredType 
							+ "* o) {" + memberName + "Cfd = o;} \\\r\n");
					outBuffer.append("    " + configuredType
							+  "* get" + memberName + "() { return (" + configuredType	+ "*) "
							+ memberName + "Cfd; }\r\n");
	
				if (!isUserspace) {
					
					outBuffer.append("#define INIT_" + className + "_" + memberName
							+ "Cfd " + property_init + "\r\n");
					
					outBuffer.append("#define NEW_" + className + "_" + memberName
							+ "Cfd " + className + "_" + memberName + "CfdCl" + "("
							+ constructor + ")\r\n");
				} else {
					outBuffer.append("#define NEW_" + className + "_" + memberName
							+ "Cfd " + className + "_" + memberName + "CfdCl" + "("
							+ constructor + ",1024," + phyloadAddr + ")\r\n");
				}
	
				} else {
					outBuffer.append("    void set" + memberName + "("
							+ configuredType + "* o) { } \\\r\n");
					outBuffer.append("    " + configuredType + "* get" + memberName
							+ "() { return 0; }\r\n");
				}
			}
		}
		
		outBuffer.append("#endif\r\n");
	}

	/**
	 * Generates a constructor parameter string out of the constructor fields saved for the given member.
	 * @param memberNode The member node which contains constructor fields
	 * @return The generated string
	 */
	private static String generateConstructorString(Element memberNode) {
		String memberClassName = memberNode.getChild("Class").getValue().trim();
		String configuredType = memberClassName.substring(memberClassName
				.lastIndexOf("/") + 1);
		Element deviceNode = memberNode.getChild("Device");
		Element addressNode = memberNode.getChild("Address");
		Element lengthNode = memberNode.getChild("Length");
		String constructor = "";

		if (deviceNode != null) {
			String device = deviceNode.getValue().trim();
			constructor += device;
		}
		if (addressNode != null) {
			String address = addressNode.getValue().trim();
			constructor += (constructor.equals("") ? "" : ",") + address;
		}
		if (configuredType.equals("MemDevice")) {
			if (lengthNode != null) {
				String length = lengthNode.getValue().trim();
				constructor += (constructor.equals("") ? "" : ",") + length;
			}
		}
		return constructor;
	}

	/**
	 * Creates a String used in the SCL confugration templates and returns it.
	 * @param templateList List of the elements the template string should contain.
	 * @return The generated string
	 */
	private static String getTemplateString(List<Element> templateList) {
		StringBuffer paramsBuffer = new StringBuffer("<");
		for (Element templateItem : templateList) {
			paramsBuffer.append(templateItem.getValue() + ", ");
		}
		String templateString = paramsBuffer.toString();
		templateString = templateString.substring(0, templateString
				.lastIndexOf(','));
		templateString += ">";
		return templateString;
	}
}