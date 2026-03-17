#ifndef ECS_TEST_H
#define ECS_TEST_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <wren.h>

void setTestVM(WrenVM* vm);
int runEcsTests(WrenVM* vm);
void ecsBindClass(const char* className, WrenForeignClassMethods* methods);
WrenForeignMethodFn ecsBindMethod(const char* signature);

#endif