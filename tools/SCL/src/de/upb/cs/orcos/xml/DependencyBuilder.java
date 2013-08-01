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

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

public class DependencyBuilder {
	
	
	
	/**
	 * 
	 * @author Matthias Grawinkel
	 *  
	 *
	 * This tool collects data from all SCLConfig.xml files given as parameters.
	 * The it writes all valid classes and members to the SCLdependencies.xml file.
	 * 
	 * Built and used out of laziness. Not used in build environment.
	 * Whether prefixes nor schemas are written to the dependencies-tag, so use it with caution!
	 * 
	 * @param args SCLdependencies.xml SCLConfig1.xml SCLConfig2.xml ...
	 * 
	 * example:
	 * scldeps.xml C:\orcos\meatz\workspace\ORCOS\configurations\prototype_01\SCLConfig.xml C:\orcos\meatz\workspace\ORCOS\configurations\localcommdemo\SCLConfig.xml C:\orcos\meatz\workspace\ORCOS\configurations\remotecommdemo\SCLConfig.xmlC:\orcos\meatz\workspace\ORCOS\configurations\virtualMemDemo\SCLConfig.xml
	 */
	public static void main(String[] args) {
		if (args.length == 0){
			System.out.println("Usage: \nParameters: SCLdependencies.xml SCLConfig1.xml SCLConfig2.xml ...");
			System.exit(1);
		}
		
		File targetFile = new File (args[0]);
		System.out.println("Writing output to: " + targetFile.getAbsolutePath());

		List<Skeleton> dependencyList = new ArrayList<Skeleton>();

		for (int i = 1; i < args.length; i++) {
			File sclConfigFile = new File (args[i]);
			System.out.println("Reading file: " + sclConfigFile.getAbsolutePath());
			try {
				readDependencies(args[i], dependencyList);
			} catch (JDOMException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		String xml = getSCLdependenciesXML(dependencyList);
		System.out.println(xml);

		FileWriter output;
		try {
			output = new FileWriter(targetFile);
			BufferedWriter bufWrite = new BufferedWriter(output);
			bufWrite.write(xml);
			bufWrite.flush();
			bufWrite.close();
		} catch (IOException e) {
			System.err.println(e.getMessage());
			System.exit(1);
		}
	}	

	private static String getSCLdependenciesXML(List<Skeleton> dependencyList) {
		
		Document document = new Document();
		
		Element rootElement = new Element("Dependencies");
		document.setRootElement(rootElement);
		
		for (Skeleton skeleton : dependencyList) {
			Element skeletonNode = new Element("Skeleton");
			
			Element nameNode = new Element("Name");
			nameNode.setText(skeleton.getName());
			skeletonNode.addContent(nameNode);
			
			//write superclasses
			if (!skeleton.getSuperclasses().isEmpty()) {
			
				Element superclassNode = new Element("Superclasses");
				for (String superClass : skeleton.getSuperclasses()) {
					Element classNode = new Element("Superclass");
					classNode.setText(superClass);
					superclassNode.addContent(classNode);
				}
				skeletonNode.addContent(superclassNode);
			}

			//write members
			if (!skeleton.getMembers().isEmpty()) {

				for (Member member : skeleton.getMembers()) {
					Element memberNode = new Element("Member");
					
					Element memberNameNode = new Element("Name");
					memberNameNode.setText(member.getName());
					memberNode.addContent(memberNameNode);

					if (!member.getClasses().isEmpty()) {
						Element classesNode = new Element("Classes");

						for (String memberClass : member.getClasses()) {
							Element classNode = new Element("Class");
							classNode.setText(memberClass);
							classesNode.addContent(classNode);
						}
						memberNode.addContent(classesNode);
					}
					skeletonNode.addContent(memberNode);	
				}
			}
			rootElement.addContent(skeletonNode);
		}

		XMLOutputter xmlOutputter = new XMLOutputter(Format.getPrettyFormat());
		return xmlOutputter.outputString(document);
	}

	/**
	 * Transforms the given xml file.
	 * @throws IOException 
	 * @throws JDOMException 
	 * 
	 */
	@SuppressWarnings("unchecked")
	private static void readDependencies(String filename, List<Skeleton> dependencyList) throws JDOMException, IOException {

		SAXBuilder saxBuilder = new SAXBuilder(false);

		Document document = saxBuilder.build(new File(filename));

		List<Element> rootChildList = document.getRootElement().getChildren();

	
		for (Element childElement : rootChildList) {
			 if (childElement.getName().equalsIgnoreCase("SCL")) {

				List<Element> sclNodes = childElement.getChildren();

				for (Element sclNode : sclNodes) {

					if (sclNode.getName().equalsIgnoreCase("Skeleton")) {
						Skeleton skeleton = null;
						String name = sclNode.getChildText("Name");	

						for (Skeleton skeletonItem : dependencyList) {

							if (skeletonItem.getName().equalsIgnoreCase(name)) {
								skeleton = skeletonItem;
								break;
							}
						}
						
						if (skeleton == null) {
							skeleton = new Skeleton();
							skeleton.setName(name);
							dependencyList.add(skeleton);
						}
							
						String superClass = sclNode.getChildText("Superclass");
						if (superClass != null) {
							skeleton.addSuperClass(superClass);
						}
						
						List<Element> memberNodes = sclNode.getChildren("Member");
						
						for (Element memberNode : memberNodes) {
							
							Member member = null;
							String memberName = memberNode.getChildText("Name");			

							for (Member memberItem : skeleton.getMembers()) {

								if (memberItem.getName().equalsIgnoreCase(memberName)) {
									member = memberItem;
									break;
								}
							}
							
							if (member == null) {
								member = new Member();
								member.setName(memberName);
								skeleton.getMembers().add(member);
							}

							String memberClass = memberNode.getChildText("Class");
							member.addClass(memberClass);
						}
					} 
				}
			}
		}
	}
}
