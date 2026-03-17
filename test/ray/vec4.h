#ifndef WREN_TEST_VEC4_H
#define WREN_TEST_VEC4_H

#include "wren.h"

WrenForeignMethodFn vec4BindMethod(const char* signature);
void vec4BindClass(const char* className, WrenForeignClassMethods* methods);

#endif // WREN_TEST_VEC4_H
