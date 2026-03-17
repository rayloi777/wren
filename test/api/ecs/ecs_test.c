#include "../api_tests.h"
#include "ecs.h"
#include "ecs_wren.h"
#include "ecs_test.h"

static WrenVM* testVm = NULL;

void setTestVM(WrenVM* vm)
{
  testVm = vm;
}

int runEcsTests(WrenVM* vm)
{
  return 0;
}

WrenForeignMethodFn ecsBindMethod(const char* signature)
{
  return ecsWrenBindMethod(signature);
}

void ecsBindClass(const char* className, WrenForeignClassMethods* methods)
{
  ecsWrenBindClass(className, methods);
}
