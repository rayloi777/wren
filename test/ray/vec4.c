#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec4.h"

static int vec4FinalizedCount = 0;

static void vec4Finalized(WrenVM* vm)
{
    wrenSetSlotDouble(vm, 0, vec4FinalizedCount);
}

static void vec4Allocate(WrenVM* vm)
{
    double* v = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);

    if (wrenGetSlotCount(vm) == 1)
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
    }
    else
    {
        v[0] = wrenGetSlotDouble(vm, 1);
        v[1] = wrenGetSlotDouble(vm, 2);
        v[2] = wrenGetSlotDouble(vm, 3);
        v[3] = wrenGetSlotDouble(vm, 4);
    }
}

static void vec4Finalize(void* data)
{
    free(data);
    vec4FinalizedCount++;
}

static void vec4GetX(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[0]);
}

static void vec4GetY(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[1]);
}

static void vec4GetZ(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[2]);
}

static void vec4GetW(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[3]);
}

static void vec4Add(WrenVM* vm)
{
    double* a = (double*)wrenGetSlotForeign(vm, 1);
    double* b = (double*)wrenGetSlotForeign(vm, 2);

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

static void vec4Sub(WrenVM* vm)
{
    double* a = (double*)wrenGetSlotForeign(vm, 1);
    double* b = (double*)wrenGetSlotForeign(vm, 2);

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

static void vec4Mul(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 1);
    double scalar = wrenGetSlotDouble(vm, 2);

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = v[0] * scalar;
    result[1] = v[1] * scalar;
    result[2] = v[2] * scalar;
    result[3] = v[3] * scalar;
}

static void vec4Div(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 1);
    double scalar = wrenGetSlotDouble(vm, 2);

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = v[0] / scalar;
    result[1] = v[1] / scalar;
    result[2] = v[2] / scalar;
    result[3] = v[3] / scalar;
}

static void vec4Dot(WrenVM* vm)
{
    double* a = (double*)wrenGetSlotForeign(vm, 0);
    double* b = (double*)wrenGetSlotForeign(vm, 1);

    double dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
    wrenSetSlotDouble(vm, 0, dot);
}

static void vec4Length(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);

    double length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    wrenSetSlotDouble(vm, 0, length);
}

static void vec4Normalize(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 1);

    double length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    if (length == 0)
    {
        double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
        result[0] = 0;
        result[1] = 0;
        result[2] = 0;
        result[3] = 0;
        return;
    }

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = v[0] / length;
    result[1] = v[1] / length;
    result[2] = v[2] / length;
    result[3] = v[3] / length;
}

static void vec4ToString(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    char result[100];
    sprintf(result, "(%g, %g, %g, %g)", v[0], v[1], v[2], v[3]);
    wrenSetSlotString(vm, 0, result);
}

WrenForeignMethodFn vec4BindMethod(const char* signature)
{
    if (strcmp(signature, "static Vec4.add(_,_)") == 0) return vec4Add;
    if (strcmp(signature, "static Vec4.sub(_,_)") == 0) return vec4Sub;
    if (strcmp(signature, "static Vec4.mul(_,_)") == 0) return vec4Mul;
    if (strcmp(signature, "static Vec4.div(_,_)") == 0) return vec4Div;
    if (strcmp(signature, "static Vec4.normalize(_)") == 0) return vec4Normalize;

    if (strcmp(signature, "static Vec4Finalized.count") == 0) return vec4Finalized;

    if (strcmp(signature, "Vec4.x") == 0) return vec4GetX;
    if (strcmp(signature, "Vec4.y") == 0) return vec4GetY;
    if (strcmp(signature, "Vec4.z") == 0) return vec4GetZ;
    if (strcmp(signature, "Vec4.w") == 0) return vec4GetW;

    if (strcmp(signature, "Vec4.add(_)") == 0) return vec4Add;
    if (strcmp(signature, "Vec4.sub(_)") == 0) return vec4Sub;
    if (strcmp(signature, "Vec4.mul(_)") == 0) return vec4Mul;
    if (strcmp(signature, "Vec4.div(_)") == 0) return vec4Div;

    if (strcmp(signature, "Vec4.dot(_)") == 0) return vec4Dot;
    if (strcmp(signature, "Vec4.length") == 0) return vec4Length;
    if (strcmp(signature, "Vec4.normalize") == 0) return vec4Normalize;
    if (strcmp(signature, "Vec4.toString") == 0) return vec4ToString;

    if (strcmp(signature, "static Vec4.add(_,_)") == 0) return vec4Add;
    if (strcmp(signature, "static Vec4.sub(_,_)") == 0) return vec4Sub;
    if (strcmp(signature, "static Vec4.mul(_,_)") == 0) return vec4Mul;
    if (strcmp(signature, "static Vec4.div(_,_)") == 0) return vec4Div;
    if (strcmp(signature, "static Vec4.dot(_,_)") == 0) return vec4Dot;
    if (strcmp(signature, "static Vec4.length(_)") == 0) return vec4Length;
    if (strcmp(signature, "static Vec4.normalize(_)") == 0) return vec4Normalize;
    if (strcmp(signature, "Vec4.toString") == 0) return vec4ToString;

    return NULL;
}

void vec4BindClass(const char* className, WrenForeignClassMethods* methods)
{
    if (strcmp(className, "Vec4") == 0)
    {
        methods->allocate = vec4Allocate;
    }
}
