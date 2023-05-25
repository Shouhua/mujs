#include "../../list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct 
{
	int count;
	struct list_head stu_list;
} klass;

typedef struct 
{
	char *name;
	unsigned short age;
	struct list_head link;
} student;

student *create_student(char *name, unsigned short age)
{
	student * stu = malloc(sizeof(student));
	stu->name = name;
	stu->age = age;
	return stu;
}

int main()
{
	klass *group = malloc(sizeof(klass));
	init_list_head(&group->stu_list);

	student *stu1 = create_student("KOBE", 33);
	list_add_tail(&stu1->link, &group->stu_list);
	student *stu2 = create_student("JAMES", 38);
	list_add_tail(&stu2->link, &group->stu_list);

	struct list_head *el, *el1;
	list_for_each_safe(el, el1, &group->stu_list)
	{
		student *e = list_entry(el, student, link);
		printf("student name: %s, age: %d\n", e->name, e->age);
	}

	return EXIT_SUCCESS;
}