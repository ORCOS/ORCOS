package de.upb.cs.orcos.xml;

import java.util.ArrayList;
import java.util.List;

class Skeleton {

	private String name;
	private List<String> superClasses = new ArrayList<String>();
	private List<Member> members = new ArrayList<Member>();

	public void addSuperClass(String superClass) {
		boolean found = false;

		for (String superClassItem : superClasses) {

			if (superClassItem.equalsIgnoreCase(superClass)) {
				found = true;
				System.out.println("found DUP! : " + superClass);
			}
		}

		if (!found) {
			superClasses.add(superClass);
		}
	}
	
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public List<String> getSuperclasses() {
		return superClasses;
	}

	public void setSuperclasses(List<String> superClasses) {
		this.superClasses = superClasses;
	}

	public List<Member> getMembers() {
		return members;
	}

	public void setMembers(List<Member> members) {
		this.members = members;
	}

	@Override
	public String toString() {
		return name;
	}
}
