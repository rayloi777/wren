#ifndef ECS_TESTS_H
#define ECS_TESTS_H

#include <stdio.h>
#include <string.h>

#include "../../../src/include/wren.h"

void ecsBindClass(const char* className, WrenForeignClassMethods* methods);
WrenForeignMethodFn ecsBindMethod(const char* signature);

#endif
