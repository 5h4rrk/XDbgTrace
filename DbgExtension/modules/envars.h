#pragma once
#include "utils/dbgextension.h"
#include "utils/resolver.h"
#include "utils/utils.h"
#include "utils/xdbg.h"

// ================ EXTERN ======================
extern std::vector<ProcessInformation> processList;
extern "C" char DomainName[0xff + 1];
extern "C" char SessionName[0xff + 1];
extern "C" char UserName[0xff + 1];

void PrintHexMemory(Buffer8 RawBuffer, u64 size) {
    for (u64 i = 0; i < size; i++) {
        Log("[%02X] ", RawBuffer[i]);
        if ((i + 1) % 0x10 == 0)
            NEWLINE();
    }
    NEWLINE();
}

// In this case: Buffer allocated is 4 bytes extra than MaxLen
// Used as search filter to find EOB (End of Buffer)
struct FormatTracker {
  u64 MaxLen;
  u64 PrevPos;
  u64 CurrPos;
  u64 MidPos;
};

// =============== FORMATTER =====================
class EnvFormatter {
  public:
    EnvFormatter() {
      this->InitTracker();
      this->setMaxLen(0x0);
    }

    void Formatter() {
      while( true){
        if (isLastEntry()) break;
        if (isEntryEnd()) {
          this->ParseEntry();
          // 42 00           00 00            Memory LayOut
          //     ^-- CurrPos
          //  Adding 2 bytes
          //                     ^--------- Alings for the next one 
          this->FmtTracker.PrevPos = this->FmtTracker.CurrPos + 0x2; 
        }
        (++this->FmtTracker.CurrPos)++; // UTF-16 (sliding by 0x2 pos)
      }
    }

    void setMaxLen(u64 e_max){
      this->FmtTracker.MaxLen = e_max;
      // if (DEBUG_MODE){
      //   TEST_LOG(this->FmtTracker.MaxLen);
      // }
    }

    void setBuffer(Buffer8 e_buffer) {
      this->e_buffer = e_buffer;
      // if (DEBUG_MODE){
      //   TEST_LOG(this->e_buffer);
      // }
    }

    void InitTracker(){
      this->FmtTracker.MaxLen &= 0x0;
      this->FmtTracker.PrevPos &= 0x0;
      this->FmtTracker.MidPos  &= 0x0;
      this->FmtTracker.CurrPos &= 0x0;
      
      // if (DEBUG_MODE) {
      //   TEST_LOG(this->FmtTracker.MaxLen);
      //   TEST_LOG(this->FmtTracker.PrevPos);
      //   TEST_LOG(this->FmtTracker.CurrPos);
      //   TEST_LOG(this->FmtTracker.MidPos);
      // }
    }

    void Reset(){
      this->InitTracker();
      this->e_buffer = nullptr;
      // if (DEBUG_MODE) {
      //   TEST_LOG(this->e_buffer);
      // }
    }

  private:
    Buffer8 e_buffer = nullptr; 
    FormatTracker FmtTracker;

    bool isLastEntry(){
      if (
      this->e_buffer[this->FmtTracker.CurrPos] == 0x0 &&
      this->e_buffer[this->FmtTracker.CurrPos + 0x1] == 0x0 &&
      this->e_buffer[this->FmtTracker.CurrPos + 0x2] == 0x0 &&
      this->e_buffer[this->FmtTracker.CurrPos + 0x3] == 0x0 ) {

        // if(DEBUG_MODE) {
        //   DbgLog(xDbg.control,"%s\n","======================");
        //   TEST_LOG(this->FmtTracker.CurrPos);
        //   TEST_LOG(this->FmtTracker.PrevPos);
        //   TEST_LOG(this->FmtTracker.MidPos);
        //   TEST_LOG(this->FmtTracker.MaxLen);
        //   DbgLog(xDbg.control,"%s\n","======================");
        // }
        this->FmtTracker.CurrPos = this->FmtTracker.MaxLen;
        this->FmtTracker.PrevPos = this->FmtTracker.CurrPos;
        return true;
      }
      return false;
    }

    bool isEntryEnd() {
      if (this->e_buffer[this->FmtTracker.CurrPos] == 0x0 && 
        this->e_buffer[this->FmtTracker.CurrPos + 0x1] == 0x0 ) {
        return true;
      }
      return false;
    }

    void findKeyValueFilter(){
      // BufferSpan where to search 
      // Exmaple:: Path=C:\Windows\System32 
      //               ^------ "="
      //        Key: Path       Value: c:\Windows\System32
      u64 sz = this->FmtTracker.CurrPos - this->FmtTracker.PrevPos; // Buffer for one entry
      for (int i=0x0; i < sz; i++){
          // NOTE: Buffer span for one entry and is of UTF-16
          // 41 00 4C 00 4C 00 55 00 53 00 45 00 52 00 53 00 50 00 52 00 4F 00 46 00 49 00 4C 00 45 00 ]-> Key
          //  [3D 00] <-------- Looking for 3D (Separator =), Marking this as MidPos
          // 43 00 3A 00 5C 00 50 00 72 00 6F 00 67 00 72 00 61 00 6D 00 44 00 61 00 74 00 61 00 ]-> Value
          if (this->e_buffer[this->FmtTracker.PrevPos + i] == 0x3d) {  // 0x3d: = 
            this->FmtTracker.MidPos = this->FmtTracker.PrevPos + i;
            i++; // UTF-16 (skipping 2 bytes)
          }
      }
    }

    void ParseEntry() {
      this->findKeyValueFilter();

      if (this->FmtTracker.MidPos == 0x0) {
        Log("Key Value Search Filter '=' not found !\n");
      }
      this->PrintKey();
      this->PrintValue();
    }

    std::string ReadTextFromMem(bool value_flag){
      u64 text_size = 0x0;
      u64 start_pos = 0x0;

      if (value_flag){
        text_size = this->FmtTracker.CurrPos - this->FmtTracker.MidPos;
        // start_pos = this->FmtTracker.MidPos + 0x2; // Skipping the seperator =
        start_pos = this->FmtTracker.MidPos;
      } else {
        text_size = this->FmtTracker.MidPos - this->FmtTracker.PrevPos;
        start_pos = this->FmtTracker.PrevPos;
      }

      u64 req_size = text_size%2 == 0  ? (text_size/2) + 0x1 : (text_size/2) + 0x2;

      // TEST_LOG(text_size);
      // TEST_LOG(req_size);
      std::vector<char> a_text(req_size);
      int j = 0;
      for(int i=0; i< text_size; i++){
          a_text[j++] = (char) this->e_buffer[start_pos + i];
          i++; // Skipping 2 bytes i.e. UTF-16
      }
      return std::string(a_text.begin(), a_text.end());
    }

    void PrintKey() {
        std::string KeyOut = this->ReadTextFromMem(false);
        DbgLog(xDbg.control, "\t%s", KeyOut.c_str());
    }

    void PrintValue() {
        std::string ValueOut = this->ReadTextFromMem(true);
        DbgLog(xDbg.control, "%s", ValueOut.c_str());
        NEWLINE();
    }
};


STDAPI _envars() {
  // Check if the array is empty
  if (std::empty(processList)) {
    LoadRequirements((std::basic_string_view<char>)PSLIST);
  }

  EnvFormatter EnvFmt;

  u64 RtlProcessParamsOffset = PebFieldOffset("ProcessParameters");
  u64 EnvSizeOffset = ProcessParamsFieldOffset("EnvironmentSize");
  u64 EnvVarOffset = ProcessParamsFieldOffset("Environment");

  ULONG bytesRead = 0x0;
  ULONG EnvSize = 0x0;
  u64 RtlProcessParams = 0x0;
  u64 Environment = 0x0;

  for (const auto &process : processList) {
   if (process.Peb == 0x0) continue;
   
    SwitchProcessContext(xDbg.client, xDbg.control, process.Eprocess);

    xDbg.dataspaces->ReadVirtual(process.Peb + RtlProcessParamsOffset, &RtlProcessParams, sizeof(&RtlProcessParams), &bytesRead);
    xDbg.dataspaces->ReadVirtual(RtlProcessParams + EnvSizeOffset, &EnvSize, sizeof(&EnvSize), &bytesRead);
    Buffer8 b = (Buffer8 )VirtualAlloc(nullptr, (size_t)((EnvSize * sizeof(u8))), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (b == NULL) {
      Log("Failed To Alloc Buffer!\n");
    }

    xDbg.dataspaces->ReadVirtual(RtlProcessParams + EnvVarOffset, &Environment, sizeof(&Environment), &bytesRead);
    xDbg.dataspaces->ReadVirtual(Environment, b,  EnvSize , &bytesRead);
    
    // TEST_LOG_STR(process.process_name);
    // TEST_LOG(process.Peb);
    // TEST_LOG(RtlProcessParams);
    // TEST_LOG(Environment);
    // TEST_LOG(EnvSize);
    DbgLog(xDbg.control, "  Process=%s\n", process.process_name);
    DbgLog(xDbg.control, "  PPID=0x%x(%d)\n", process.PPID, process.PPID);
    DbgLog(xDbg.control, "  PID=0x%x(%d)\n", process.PID, process.PID);
    DbgLog(xDbg.control, "  Peb=0x%p\n", process.Peb);
    DbgLog(xDbg.control, "  Params=0x%p\n", RtlProcessParams);
    DbgLog(xDbg.control, "  Environment=0x%p\n", Environment);
    DbgLog(xDbg.control, "  EnvironmentSize=0x%x\n", EnvSize);


    EnvFmt.setMaxLen(EnvSize);
    EnvFmt.setBuffer(b);
    EnvFmt.Formatter();
    EnvFmt.Reset();
   
     StatusCode s_code = VirtualFree(b, EnvSize, MEM_DECOMMIT);
     if (!s_code) {
         Log("VirtualFree (DECOMMIT) failed: %x\n", GetLastError());
     } else {
        //  Log("Successfully decommitted buffer [.....] : 0x%p\n", b);
         Log("\n");
     }
  }
  return S_OK;
}
