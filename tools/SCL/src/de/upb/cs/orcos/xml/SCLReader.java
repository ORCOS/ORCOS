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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

import javax.xml.XMLConstants;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.jdom.Attribute;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;
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
    
    private static StringBuffer makeDefines = new StringBuffer();

    private static StringBuffer tasksprioritybuffer = new StringBuffer();
    private static StringBuffer tasksphasebuffer = new StringBuffer();
    private static StringBuffer tasksperiodbuffer = new StringBuffer();
    private static StringBuffer tasksdeadlinedbuffer = new StringBuffer();
    private static StringBuffer tasksexectimebuffer = new StringBuffer();

    private static StringBuffer tasksnamebuffer = new StringBuffer();

    private static ArrayList<String> typedefs = new ArrayList();

    // incremented for each task that is found in the config file. this number
    // is written as first entry to the DEFAULT_TARGET_TASK_CONFIG_NAME
    private static int numberOfTasks = 0;

    private static int verboselevel = 3;

    /**
     * Main method which is called on program startup.
     * @param args The command line arguments passed to the program
     */
    public static void main(String[] args) {

        if (args.length < 2) {
            System.out.println("Usage: path-to-xml-file path-to-xml-schema-file [--output-dir path-to-output-dir][--check-deps path-to-deps-xml-file path-to-deps-schema-file]");
            System.exit(1);
        }

        String xmlInputFileName  = args[0];
        String xmlSchemaFileName = args[1];

        String dependencyXML = "";
        String dependencySchema = "";

        // check whether the configfile should be validated against the deps-xml
        boolean checkAgainstDeps = false;

        for (int i = 0; i < args.length; i++) {

            if (args[i].equals("--generate-deps")) {
                 dependencyXML = args[++i];
                 dependencySchema = args[++i];

                  try {
                      xIncludeDep(new File(dependencyXML).toPath(),new File(dependencyXML).getParent());

                      if (validate(dependencySchema, dependencyXML)) {
                          if (verboselevel > 0)
                              System.out.println("SCL Dependencies Definition is valid.");
                          checkAgainstDeps = true;
                      }
                 } catch (Exception e) {
                      System.err.println("Error generting Dependencies File " + dependencyXML);
                      System.err.println(e.getMessage());
                      System.exit(1);
                 }
                 System.exit(0);
            }

            if (args[i].equals("--check-deps")) {
                dependencyXML = args[++i];
                dependencySchema = args[++i];

                try {
                    if (verboselevel > 0)
                        System.out.println("Checking against dependencies file: "  + new File(dependencyXML).getAbsolutePath());

                    if (validate(dependencySchema, dependencyXML)) {
                        if (verboselevel > 0)
                        System.out.println("SCL Dependencies Definition is valid.");
                        checkAgainstDeps = true;
                    }
                } catch (Exception e) {
                    System.err.println("Error while validating SCL Dependencies Definition " + dependencySchema);
                    System.err.println(e.getMessage());
                    System.exit(1);
                }
            } else if (args[i].equals("--output-dir")) {
                outputDir = new File(args[++i]);
                if (!outputDir.canWrite()) {
                    System.err.println("Target output-dir is either illegal or not writable");
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
        sclConfigBuffer.append("#ifndef __ASSEMBLER__\r\n #include \"types.hh\"\r\n#endif\r\n");

        // This is written to tasktable.S
        StringBuffer taskTableBuffer = new StringBuffer();
        tasksnamebuffer.append(".section .data\n");

         tasksdirbuffer.append("TASKS=(");
         tasksstartbuffer.append("TASKS_START=(");
         tasksendbuffer.append("TASKS_SIZE=(");
         tasksprioritybuffer.append("TASKS_PRIORITY=(");
         tasksphasebuffer.append("TASKS_PHASE=(");
         tasksperiodbuffer.append("TASKS_PERIOD=(");
         tasksdeadlinedbuffer.append("TASKS_DEADLINE=(");
         tasksexectimebuffer.append("TASKS_EXECTIME=(");

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
        tasksprioritybuffer.append(")\n");
        tasksphasebuffer.append(")\n");
        tasksperiodbuffer.append(")\n");
        tasksdeadlinedbuffer.append(")\n");
        tasksexectimebuffer.append(")\n");

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
            //bufWrite.write(tasksdirbuffer.toString());
            bufWrite.write(tasksstartbuffer.toString());
            bufWrite.write(tasksendbuffer.toString());
            bufWrite.write(tasksprioritybuffer.toString());
            bufWrite.write(tasksphasebuffer.toString());
            bufWrite.write(tasksperiodbuffer.toString());
            bufWrite.write(tasksdeadlinedbuffer.toString());
            bufWrite.write(tasksexectimebuffer.toString());

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

            bufWrite.write(makeDefines.toString());
            
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
            bufWrite.write(tasksnamebuffer.toString());
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

    @SuppressWarnings("unchecked")
    protected static Element getChildByNameTextValue(Element parent, String child, String textValue) {
        if (parent != null) {
            List<Element> mainElementsList = parent.getChildren(child);
            for (int i = 0; i < mainElementsList.size(); i++) {
                Element element = (Element) (mainElementsList.get(i));
                 if (element != null) {
                     String elementName = element.getChildText("Name");
                     if (elementName != null && elementName.equals(textValue)) {
                         return element;
                     }
                 }
            }
        }
        return null;
    }

    public static void xIncludeDep(Path depfile, String archDir) throws Exception {

         System.out.println("Parsing " + archDir + " for architecture xml files.");
         File folder = new File(archDir);

         String inFile = depfile.getParent().toString() + "/SCLBase.xml";

         SAXBuilder saxBuilder = new SAXBuilder(false);
         Document document = saxBuilder.build(new File(inFile));

         Element dep = document.getRootElement();
         Element board = getChildByNameTextValue(dep,"Skeleton","Board");

        if (board == null) {
            System.out.println("No Board Skeleton found.. aborting");
            System.exit(3);
        }


         for (final File fileEntry : folder.listFiles()) {
             if (fileEntry.getName().toLowerCase().equals("sclbase.xml") ||
                 fileEntry.getName().toLowerCase().equals("scldependencies.xml")) continue;

             if (fileEntry.isDirectory() == false) {
                 System.out.println("\nIncluding " + fileEntry.getName());

                 SAXBuilder saxBuilder2 = new SAXBuilder(false);
                 try {
                     Document document2 = saxBuilder2.build(fileEntry);
                     Element root = document2.getRootElement();
                     Element boardElement = getChildByNameTextValue(root,"Skeleton","Board");
                     List<Element> superclasses = root.getChildren("Superclasses");
                     for (Element e : superclasses) {
                         System.out.println("  Adding Board Definitions");
                         board.getChild("Superclasses").addContent(e.cloneContent());
                     }
                     // add all devices defined inside the file
                     List<Element> devices = root.getChildren("Device");
                     if (devices.size() > 0)
                         System.out.print("  Adding Devices Definitions:\n  ");

                     for(Element device : devices) {
                         String[] names = device.getAttributeValue("Name").split(",");
                         for (String name : names) {
                              Element memberElement = getChildByNameTextValue(board,"Member",name);
                              if (memberElement != null) {
                                  System.out.print(name + " ");
                                  memberElement.getChild("Classes").addContent(device.cloneContent());
                              } else {
                                  // Create the element
                                  Element e = new Element("Member");
                                  System.out.print(name + " ");
                                  e.addContent(new Element("Name").setText(name));
                                  Element e_classes = new Element("Classes");
                                  e_classes.addContent(device.cloneContent());
                                  e.addContent(e_classes);
                                  board.addContent(e);
                              }
                         }
                     }

                 } catch (Exception e) {
                       System.out.println("Ignoring file due to exception: " + e.getMessage());
                 }
               }
         }


         XMLOutputter xmlOutput = new XMLOutputter();

         // display nice nice
         xmlOutput.setFormat(Format.getPrettyFormat());
         System.out.println("\nWriting final Dependency file: " + depfile.toString());

         xmlOutput.output(document, new FileWriter(depfile.toString()));
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

        SAXBuilder saxBuilder       = new SAXBuilder(false);
        Document document           = saxBuilder.build(new File(filename));
        List<Element> rootChildList = document.getRootElement().getChildren();

        // these variables are only created and later used if --check-deps
        // parameter was supplied
        Document dependenciesDocument;
        List<Element> dependenciesList = null;

        if (checkDeps) {
            dependenciesDocument = saxBuilder.build(new File(depsFile));
            dependenciesList     = dependenciesDocument.getRootElement().getChildren();
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

                                    if (checkSkeletonDependencies(sclNode, dependencyElement)) {
                                        // the current node is valid against its
                                        // dependencies
                                        processSkeletonNode(sclNode, configBuffer);
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

        Element depSuperClassElement = null;
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
                    depSuperClassElement = depSuperclassItem;
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
        List<Element> mandatoryDepMember =  depSuperClassElement.getChildren("MemberRef");

        // check if every dependent member has a configuration!
        for (Element depMember : mandatoryDepMember) {
            //System.out.println("MemberRef: " + depMember.getAttributeValue("Name").trim());
            String depMemberName = depMember.getAttributeValue("Name").trim();
            boolean found = false;
            for (Element member : memberList) {
                if (member.getAttributeValue("Name").trim().equals(depMemberName)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (skeletonName.equals("SyscallManager")) {
                    // not configuring syscalls is ok and just triggers a warning
                    System.out.println("Warning: Syscall '" + depMemberName + "' not found in configuration. Assuming unavailable!");
                    Element sc = new Element("Member");
                    skeleton.addContent(sc);
                    sc.setAttribute("Name", depMemberName);
                    sc.setAttribute("None","true");
                } else {
                    System.err.println("Error: Mandatory member '" + depMemberName + "' of Skeleton " + skeletonName + " not found in configuration!");
                    return false;
                }
            }
        }

        // check if every configured member is valid!
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
                            if (verboselevel > 4)
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
            // add networking objects.. this could be managed inside the files
            // but we want compiling to be fast so we avoid touching them if
            // networking is disabled
            arch_objects.add("Socket.o");
            arch_objects.add("ProtocolPool.o");
            arch_objects.add("TCPTransportProtocol.o");
            arch_objects.add("IPv4AddressProtocol.o");
            arch_objects.add("UDPTransportProtocol.o");
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
            arch_objects.add("dns.o");
            arch_objects.add("icmp.o");
            arch_objects.add("icmp6.o");
            arch_objects.add("ethar.o");
            arch_objects.add("ethndp.o");
            arch_objects.add("FixedSizePBufList.o");
        }

        // add workertask files
        if (defineNode.getChildText("Name").equals("USE_WORKERTASK") && defineNode.getChildText("Value").equals("1"))
        {
            arch_objects.add("KernelThread.o");
            arch_objects.add("KernelTask.o");
        }

    }

    /**
     * Appends a #define to the outbuffer
     *
     * @param defineNode
     * @param outBuffer
     */
    private static void processTaskNode(Element taskNode,
            StringBuffer taskBuffer) throws Exception  {

        String start = taskNode.getChildText("Start");
        if (start != null) {
            taskBuffer.append(".long " + start + ";\n");
        } else {
            throw new Exception("Task: " + taskNode + " had no start address");
        }

        numberOfTasks++;
        tasksbuffer.append(taskNode.getChildText("Path") + " ");
        tasksdirbuffer.append(taskNode.getChildText("Path") + " ");
        tasksstartbuffer.append(taskNode.getChildText("Start") + " ");
        tasksendbuffer.append(taskNode.getChildText("Size") + " ");

        /* append size of memory area reserved for this initial task */
        taskBuffer.append(".long " + taskNode.getChildText("Size") + ";\n");
        /* append pointer to name */
        taskBuffer.append(".long task" + numberOfTasks + "_name;\n");

        /* get the task name*/
        String taskname = "Task_" + numberOfTasks;

        String filepath = taskNode.getChildText("Path");
        try {
            String dirname = new File(filepath).getName();
            taskname = dirname;
        } catch (Exception e) {

        }

        tasksnamebuffer.append("task"  + numberOfTasks + "_name:\n");
        tasksnamebuffer.append(".ascii \"" + taskname + "\\0\"\n");

        // get PriorityOptions
        Element priorityOptions = taskNode.getChild("PriorityOptions");

        if (priorityOptions != null) {

            String initialPriority = priorityOptions.getChildText("InitialPriority");
            if (initialPriority != null) {
                tasksprioritybuffer.append(initialPriority + " ");
            } else {
                tasksprioritybuffer.append("0 ");
            }

            String phase = priorityOptions.getChildText("Phase");
            if (phase != null) {
                tasksphasebuffer.append(phase + " ");
            } else {
                tasksphasebuffer.append("0 ");
            }

            // get <RealTimeOptions>
            Element realTimeOptions = priorityOptions.getChild("RealTimeOptions");

            if (realTimeOptions != null) {
                String period = realTimeOptions.getChildText("Period");

                if (period != null) {
                    tasksperiodbuffer.append(period + " ");
                } else {
                    tasksperiodbuffer.append("0 ");
                }

                String deadline = realTimeOptions.getChildText("Deadline");
                if (deadline != null) {
                    tasksdeadlinedbuffer.append(deadline + " ");
                } else {
                    tasksdeadlinedbuffer.append("0 ");
                }

                String executionTime = realTimeOptions.getChildText("ExecutionTime");
                if (executionTime != null) {
                    tasksexectimebuffer.append(executionTime + " ");

                } else {
                    tasksexectimebuffer.append("0 ");
                }
            }
        } else {
            // write default values for period, deadline, exectime
            tasksprioritybuffer.append("0 ");
            tasksphasebuffer.append("0 ");
            tasksperiodbuffer.append("0 ");
            tasksdeadlinedbuffer.append("0 ");
            tasksexectimebuffer.append("0 ");
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
        String className = skeletonNode.getChild("Name").getValue().trim();
        outBuffer.append("\r\n// Configuration of class " + className + "\r\n #ifndef __ASSEMBLER__\r\n");

        String superClassName = null;

        // write superclass configuration to out buffer
        Element superClassNode = skeletonNode.getChild("Superclass");
        if (superClassNode != null) {
            superClassName = superClassNode.getValue().trim();
            outBuffer.append("#define " + className + "Cfd_hh <"
                    + superClassName + ".hh>\r\n");
            outBuffer.append("#define "    + className    + "CfdCl "
                    + superClassName.substring(superClassName.lastIndexOf("/") + 1)
                    + templateString + "\r\n");
        }

        if (className.equals("Board")) {
            // check superclass and set ARCH_DIR appropriately
            archdirbuffer.append("ARCH_DIR = $(KERNEL_DIR)");
            String s = superClassName.substring(0, superClassName.lastIndexOf("/"));
            archdirbuffer.append(s);
            archdirbuffer.append("/\n");
            if (verboselevel > 2) System.out.println("ARCH_DIR: " + s);

        }

        List<Element> memberNodes = skeletonNode.getChildren("Member");
        for (Element memberNode : memberNodes) {

            String memberName = memberNode.getAttributeValue("Name").trim();
            String memberClassName = memberNode.getAttributeValue("Class","").trim();


            if (verboselevel > 2) {
                if (memberClassName.isEmpty())
                    System.out.printf("Processing %-30s ",memberName);
                else
                    System.out.printf("Processing %-30s [%s]", memberName, memberClassName);
            }


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
                        //System.out.println("Warning: not adding " + memberClassFileName + " to object list (Dummy* Filter)!");
                    }

                }
            }

            if (!isNone && !className.equals("SyscallManager")){
                // this class is configured so we must compile it
                if (!memberClassFileName.contains("Dummy")) {
                    arch_objects.add(memberClassFileName + ".o");
                }
            }
        

            if (className.equals("Kernel") && !isNone) {

                if (memberName.equals("Thread")) {
                    if (memberClassFileName.contains("RealTimeThread"))
                        arch_objects.add("PriorityThread.o");

                }

            }


            outBuffer.append("\r\n// Configuration of member " + memberName
                    + " of class " + className + "\r\n");

            if (className.equals("SyscallManager")){
                // Syscall manager syscall definition
                if (!isNone) {
                    /* declare the syscall method */
                    outBuffer.append("int sc_" + memberName + "(intptr_t sp_int);\r\n");
                    System.out.print(" [Syscall available]");
                } else {
                    /* redirect syscall handler to default implementation */
                    outBuffer.append("#define sc_" + memberName + " sc_default_handler\r\n");
                    System.out.print(" [Syscall unavailable]");
                }
            }

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

                makeDefines.append("HAS_" + className + "_" + memberName + "Cfd := 1\n");
                
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
                            + "CfdCl " + configuredType    + "\r\n");
                    outBuffer.append("#define " + className + "_" + memberName
                            + "CfdT " + configuredType + "*\r\n");
                } else {
                    configuredType = "USCommDeviceDriver";

                    outBuffer.append("#define " + className + "_" + memberName
                            + "_hh <hal/USCommDeviceDriver.hh>\r\n");
                    outBuffer.append("#define " + className + "_" + memberName
                            + "_cc <hal/USCommDeviceDriver.cc>\r\n");
                    outBuffer.append("#define " + className + "_" + memberName
                            + "CfdCl " + configuredType     + "\r\n");
                    outBuffer.append("#define " + className + "_" + memberName
                            + "CfdT " + configuredType + "*\r\n");
                }



                // Generate Member definition
                outBuffer.append("#define DEF_" + className + "_" + memberName
                        + "Cfd \\\r\n");

                if (!isNone) {
                    outBuffer.append("private: \\\r\n");
                    outBuffer.append("    " + configuredType+  "* "    + memberName + "Cfd; \\\r\n");
                }

                outBuffer.append("public: \\\r\n");

                if (!isNone) {

                    outBuffer.append("    void set" + memberName + "("
                            + configuredType
                            + "* o) {" + memberName + "Cfd = o;} \\\r\n");
                    outBuffer.append("    " + configuredType
                            +  "* get" + memberName + "() { return (" + configuredType    + "*) "
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

            System.out.println(" ");
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