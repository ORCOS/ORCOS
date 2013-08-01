package de.upb.cs.orcos.xml;

import java.util.ArrayList;
import java.util.List;

class Member {

	private String name;
	private List<String> classes = new ArrayList<String>();

	public void addClass(String memberClass) {
		boolean found = false;

		for (String memberClassItem : classes) {

			if (memberClassItem.equalsIgnoreCase(memberClass)) {
				found = true;
				System.out.println("found DUP! : " + memberClass );
			}
		}

		if (!found) {
			classes.add(memberClass);
		}
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	public List<String> getClasses() {
		return classes;
	}
	public void setClasses(List<String> classes) {
		this.classes = classes;
	}
	
	@Override
	public String toString() {
		return name;
	}	
}
