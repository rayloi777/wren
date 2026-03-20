// ============================================================================
// game_test/main.c - Wren 遊戲引擎示範
//
// 功能：展示如何用 C 載入外部 .wren 檔案並執行遊戲循環
// 支援：Ctrl+C 優雅退出 | 高精度計時 | 固定時間步長
//
// 使用方式：
//   1. make clean && make
//   2. ./game_test
//   3. 按 Ctrl+C 退出
// ============================================================================

// ============================================================================
// 標準庫頭文件
// ============================================================================
#include <stdio.h>      // printf, fprintf, snprintf
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strncpy, memset
#include <time.h>       // clock_t (備用)

// ============================================================================
// 系統頭文件
// ============================================================================
#include <libgen.h>     // dirname - 取得執行檔所在目錄
#include <mach/mach_time.h>  // mach_absolute_time - 高精度計時（macOS）
#include <signal.h>     // signal, SIGINT, SIGTERM - 信號處理

// ============================================================================
// Wren API
// ============================================================================
#include "wren.h"       // Wren VM, WrenInterpretResult, WrenHandle, etc.

// ============================================================================
// 巨集定義 - 遊戲配置參數
// ============================================================================
#define MAX_PATH 1024              // 檔案路徑緩衝區大小
#define TARGET_FPS 60             // 目標幀率
#define FIXED_DT (1.0 / TARGET_FPS)  // 固定時間步長 ≈ 16.67ms

// ============================================================================
// 全域狀態
// ============================================================================
static char exeDir[MAX_PATH] = ".";      // 執行檔所在目錄（用於定位 .wren 檔）
static volatile int running = 1;        // 遊戲循環標誌（=0 時退出）

// ============================================================================
// signalHandler - 信號處理函數
//
// 用途：處理 Ctrl+C (SIGINT) 和 kill (SIGTERM) 信號
// 效果：將 running 設為 0，使遊戲循環優雅退出
// ============================================================================
static void signalHandler(int sig)
{
  (void)sig;  // 抑制未使用警告
  running = 0;
}

// ============================================================================
// getTime - 高精度計時器（macOS）
//
// 原理：
//   1. mach_absolute_time() 取得 CPU 時鐘週期數
//   2. mach_timebase_info() 取得轉換比率（numer/denom）
//   3. 週期數 × 轉換比率 / 1e9 = 實際時間（秒）
//
// 優點：
//   - 精度遠高於 clock()（可達到奈秒級）
//   - 不受系統負載影響
//   - 適合遊戲等需要精確時間測量的場景
// ============================================================================
static double getTime()
{
  // 靜態變數：時間基準（只初始化一次）
  static mach_timebase_info_data_t timebase;
  
  // 初始化：取得時間基準（numer/denom 通常是 1/1 或 125/1）
  if (timebase.denom == 0)
  {
    mach_timebase_info(&timebase);
  }
  
  // 取得 CPU 週期數
  uint64_t time = mach_absolute_time();
  
  // 轉換為秒：週期數 × (numer/denom) / 1e9
  return (double)time * timebase.numer / timebase.denom / 1e9;
}

// ============================================================================
// writeFn - Wren 輸出回調
//
// 用途：當 Wren 程式執行 System.print() 時呼叫此函數
// 參數：
//   - vm: Wren VM 指標（本函數未使用）
//   - text: 要輸出的文字
// ============================================================================
static void writeFn(WrenVM* vm, const char* text)
{
  (void)vm;  // 抑制未使用警告
  printf("%s", text);
}

// ============================================================================
// errorFn - Wren 錯誤處理回調
//
// 用途：當 Wren 發生編譯錯誤、運行時錯誤或棧追蹤時呼叫此函數
// 參數：
//   - vm: Wren VM 指標（本函數未使用）
//   - errorType: 錯誤類型（編譯/運行時/棧追蹤）
//   - module: 模組名稱
//   - line: 錯誤行號
//   - msg: 錯誤訊息
// ============================================================================
static void errorFn(WrenVM* vm, WrenErrorType errorType,
                    const char* module, const int line,
                    const char* msg)
{
  (void)vm;  // 抑制未使用警告
  
  switch (errorType)
  {
    case WREN_ERROR_COMPILE:
      // 編譯錯誤：語法錯誤、解析錯誤等
      printf("[%s line %d] [Error] %s\n", module, line, msg);
      break;
    case WREN_ERROR_STACK_TRACE:
      // 棧追蹤：運行時錯誤的調用棧
      printf("[%s line %d] in %s\n", module, line, msg);
      break;
    case WREN_ERROR_RUNTIME:
      // 運行時錯誤：Fiber.abort()、空引用等
      printf("[Runtime Error] %s\n", msg);
      break;
  }
}

// ============================================================================
// readFile - 讀取檔案內容
//
// 參數：
//   - path: 檔案路徑
// 返回值：
//   - 成功：包含檔案內容的字串指標（需要 free）
//   - 失敗：NULL
//
// 注意：返回的字串需要調用者手動 free()
// ============================================================================
static char* readFile(const char* path)
{
  // 以二進制模式開啟檔案
  FILE* file = fopen(path, "rb");
  if (file == NULL)
  {
    return NULL;
  }

  // 取得檔案大小
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // 配置緩衝區（+1 給結尾的 \0）
  char* buffer = (char*)malloc(size + 1);
  if (buffer == NULL)
  {
    fclose(file);
    return NULL;
  }

  // 讀取檔案內容
  size_t bytesRead = fread(buffer, 1, size, file);
  buffer[bytesRead] = '\0';
  fclose(file);

  return buffer;
}

// ============================================================================
// onModuleComplete - 模組卸載回調
//
// 用途：當 Wren 載入模組完成後，回調此函數釋放資源
// 原理：loadModule 回調中將 source 字串設為 userData
//       此回調被呼叫時釋放該記憶體
// ============================================================================
static void onModuleComplete(WrenVM* vm, const char* name, WrenLoadModuleResult result)
{
  (void)vm;   // 抑制未使用警告
  (void)name;  // 抑制未使用警告
  
  // 釋放 loadModule 中分配的記憶體
  if (result.userData != NULL)
  {
    free(result.userData);
  }
}

// ============================================================================
// loadModule - 模組載入回調
//
// 用途：當 Wren 執行 import "xxx" 時，回調此函數載入 .wren 檔
// 參數：
//   - vm: Wren VM 指標
//   - name: 模組名稱（如 "game"）
// 返回值：
//   - WrenLoadModuleResult結構體，包含：
//     - source: 檔案內容（必須調用 free()）
//     - userData: 用戶資料指標（這裡設為 source 字串）
//     - onComplete: 完成回調（釋放 userData）
// ============================================================================
static WrenLoadModuleResult loadModule(WrenVM* vm, const char* name)
{
  WrenLoadModuleResult result = { NULL, NULL, NULL };

  // 構造完整路徑：exeDir/game.wren
  char path[MAX_PATH];
  snprintf(path, sizeof(path), "%s/%s.wren", exeDir, name);

  // 讀取檔案
  char* source = readFile(path);
  if (source != NULL)
  {
    result.source = source;              // 原始碼內容
    result.userData = source;           // 用戶資料指標
    result.onComplete = &onModuleComplete;  // 完成後釋放資源
  }

  return result;
}

// ============================================================================
// main - 程式入口
//
// 流程：
//   1. 初始化階段 - 取得執行檔目錄、註冊信號處理
//   2. 載入階段 - 創建 VM、載入遊戲模組
//   3. 準備階段 - 取得遊戲物件、創建方法句柄
//   4. 遊戲循環 - 固定時間步長更新
//   5. 清理階段 - 釋放資源
// ============================================================================
int main(int argc, char* argv[])
{
  // ------------------------------------------------------------------------
  // 初始化階段
  // ------------------------------------------------------------------------
  
  // 取得執行檔所在目錄
  // 用途：從任意目錄執行都能正確找到 .wren 檔
  // 例如：/path/to/game_test 可找到 /path/to/game.wren
  strncpy(exeDir, dirname(argv[0]), sizeof(exeDir) - 1);
  exeDir[sizeof(exeDir) - 1] = '\0';

  // 註冊信號處理器 - 支援 Ctrl+C 和 kill 優雅退出
  signal(SIGINT, signalHandler);   // Ctrl+C
  signal(SIGTERM, signalHandler);  // kill

  // 初始化 Wren 配置
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.writeFn = &writeFn;           // 輸出回調
  config.errorFn = &errorFn;           // 錯誤回調
  config.loadModuleFn = &loadModule;   // 模組載入回調

  // 創建 VM
  WrenVM* vm = wrenNewVM(&config);
  if (vm == NULL)
  {
    printf("Failed to create VM\n");
    return 1;
  }

  // ------------------------------------------------------------------------
  // 載入遊戲模組
  // ------------------------------------------------------------------------
  
  // 執行 import "game"
  // 會觸發 loadModule 回調，嘗試載入 game.wren
  WrenInterpretResult result = wrenInterpret(vm, "main", "import \"game\"");
  if (result != WREN_RESULT_SUCCESS)
  {
    printf("Failed to load game module\n");
    wrenFreeVM(vm);
    return 1;
  }

  // 取得全域變數 game（遊戲類別實例）
  // slot 0 將包含 game 物件
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "game", "game", 0);
  WrenHandle* gameHandle = wrenGetSlotHandle(vm, 0);
  if (gameHandle == NULL)
  {
    printf("Error: Could not get game variable\n");
    wrenFreeVM(vm);
    return 1;
  }

  // ------------------------------------------------------------------------
  // 創建方法句柄
  //
  // 注意：句柄創建一次，重複使用
  // 方法簽名格式：
  //   - "init()"      - 無參數方法
  //   - "loop(_)"     - 一個參數（下劃線表示）
  //   - "render(_)"    - 一個參數
  // ------------------------------------------------------------------------
  
  WrenHandle* initHandle = wrenMakeCallHandle(vm, "init()");
  WrenHandle* updateHandle = wrenMakeCallHandle(vm, "update(_)");
  WrenHandle* renderHandle = wrenMakeCallHandle(vm, "render(_)");

  if (initHandle == NULL || updateHandle == NULL || renderHandle == NULL)
  {
    printf("Error: Could not create method handles\n");
    wrenReleaseHandle(vm, gameHandle);
    wrenFreeVM(vm);
    return 1;
  }

  // ------------------------------------------------------------------------
  // 調用遊戲初始化
  //
  // 設置：
  //   - slot 0: 遊戲物件（receiver）
  // 調用 init() 初始化遊戲狀態
  // ------------------------------------------------------------------------
  
  wrenEnsureSlots(vm, 2);
  wrenSetSlotHandle(vm, 0, gameHandle);
  result = wrenCall(vm, initHandle);
  if (result != WREN_RESULT_SUCCESS)
  {
    printf("Error calling init()\n");
    goto cleanup;
  }

  // ------------------------------------------------------------------------
  // 遊戲主循環 - 固定時間步長（Fixed Timestep）
  //
  // 架構：
  //   while (running) {
  //     dt = getTime() - lastTime      // 計算實際流逝時間
  //     accumulator += dt                 // 累加流逝時間
  //     
  //     while (accumulator >= FIXED_DT) {  // 固定時間步長循環
  //       loop(FIXED_DT)                 // 遊戲邏輯更新
  //       render(FIXED_DT)                // 渲染（預留接口）
  //       accumulator -= FIXED_DT
  //     }
  //   }
  //
  // 優點：
  //   - 固定時間步長，遊戲邏輯與硬體無關
  //   - 防止高速設備上遊戲速度過快
  //   - 方便網路同步和物理模擬
  //
  // 注意：dt 參數固定為 FIXED_DT，確保遊戲邏輯一致性
  // ------------------------------------------------------------------------
  
  double lastTime = getTime();
  double accumulator = 0.0;

  while (running)
  {
    // 計算實際流逝時間
    double currentTime = getTime();
    double dt = currentTime - lastTime;
    lastTime = currentTime;

    accumulator += dt;

    // 固定時間步長循環
    // 確保每幀更新次數與目標 FPS 一致
    while (accumulator >= FIXED_DT)
    {
      // 調用 update(FIXED_DT) - 遊戲邏輯更新
      // slot 0: 遊戲物件
      // slot 1: dt 參數
      wrenSetSlotHandle(vm, 0, gameHandle);
      wrenSetSlotDouble(vm, 1, FIXED_DT);
      wrenCall(vm, updateHandle);

      // 調用 render(FIXED_DT) - 渲染
      // 預留接口，未來可接入 GPU 渲染
      wrenSetSlotHandle(vm, 0, gameHandle);
      wrenSetSlotDouble(vm, 1, FIXED_DT);
      wrenCall(vm, renderHandle);

      accumulator -= FIXED_DT;
    }
  }

  printf("\nGame loop ended cleanly\n");

  // ------------------------------------------------------------------------
  // 清理階段 - 釋放所有資源
  //
  // 使用 goto cleanup 統一出口，方便資源釋放
  // 順序：句柄 → VM
  // ------------------------------------------------------------------------
cleanup:
  wrenReleaseHandle(vm, gameHandle);
  wrenReleaseHandle(vm, initHandle);
  wrenReleaseHandle(vm, updateHandle);
  wrenReleaseHandle(vm, renderHandle);
  wrenFreeVM(vm);

  return 0;
}