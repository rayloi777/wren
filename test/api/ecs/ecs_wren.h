#ifndef ECS_WREN_H
#define ECS_WREN_H

#include <wren.h>

void ecsWrenBindClass(const char* className, WrenForeignClassMethods* methods);
WrenForeignMethodFn ecsWrenBindMethod(const char* signature);

#endif
