/* SPDX-License-Identifier: BSD-2-Clause */

#include <tilck/common/basic_defs.h>
#include <tilck/common/printk.h>

#include <tilck/kernel/sched.h>
#include <tilck/kernel/timer.h>
#include <tilck/kernel/datetime.h>

#include <3rd_party/acpi/acpi.h>
#include <3rd_party/acpi/acpiosxf.h>
#include <3rd_party/acpi/acexcep.h>

ACPI_STATUS
AcpiOsInitialize(void)
{
   printk("AcpiOsInitialize\n");
   return AE_OK;
}

ACPI_STATUS
AcpiOsTerminate(void)
{
   printk("AcpiOsTerminate\n");
   return AE_OK;
}

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void)
{
   ACPI_PHYSICAL_ADDRESS ptr = 0;
   AcpiFindRootPointer(&ptr);
   return ptr;
}

ACPI_STATUS
AcpiOsPredefinedOverride(
    const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING                 *NewVal)
{
   *NewVal = NULL;
   return AE_OK;
}

ACPI_STATUS
AcpiOsTableOverride(
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable)
{
   *NewTable = NULL;
   return AE_OK;
}

ACPI_STATUS
AcpiOsPhysicalTableOverride(
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_PHYSICAL_ADDRESS   *NewAddress,
    UINT32                  *NewTableLength)
{
   *NewAddress = 0;
   return AE_OK;
}

void
AcpiOsSleep(UINT64 Milliseconds)
{
   kernel_sleep_ms(Milliseconds);
}

void
AcpiOsStall(UINT32 Microseconds)
{
   disable_preemption();
   {
      delay_us(Microseconds);
   }
   enable_preemption_nosched();
}

UINT64
AcpiOsGetTimer(void)
{
   /* system time in 100-ns units */
   return get_sys_time() / 100;
}

ACPI_THREAD_ID
AcpiOsGetThreadId(void)
{
   return get_curr_tid();
}

ACPI_STATUS
AcpiOsExecute(
    ACPI_EXECUTE_TYPE       Type,
    ACPI_OSD_EXEC_CALLBACK  Function,
    void                    *Context)
{
   int prio;

   if (!Function)
      return AE_BAD_PARAMETER;

   switch (Type) {

      case OSL_NOTIFY_HANDLER:
      case OSL_GPE_HANDLER:
         prio = 1;
         break;

      case OSL_DEBUGGER_MAIN_THREAD:
      case OSL_DEBUGGER_EXEC_THREAD:
         prio = 3;
         break;

      case OSL_GLOBAL_LOCK_HANDLER: /* fall-through */
      case OSL_EC_POLL_HANDLER:     /* fall-through */
      case OSL_EC_BURST_HANDLER:    /* fall-through */
      default:
         prio = 2;
         break;
   }

   if (!wth_enqueue_anywhere(prio, Function, Context)) {
      if (!wth_enqueue_anywhere(WTH_PRIO_LOWEST, Function, Context))
         panic("AcpiOsExecute: unable to enqueue job");
   }

   return AE_OK;
}

ACPI_STATUS
AcpiOsSignal (
    UINT32                  Function,
    void                    *Info)
{
   if (Function == ACPI_SIGNAL_FATAL) {

      ACPI_SIGNAL_FATAL_INFO *i = Info;

      panic("ACPI: fatal signal. Type: 0x%x, Code: 0x%x, Arg: 0x%x\n",
            i->Type, i->Code, i->Argument);

   } else if (Function == ACPI_SIGNAL_BREAKPOINT) {

      printk("ACPI: breakpoint: %s\n", (char *)Info);
      printk("ACPI: ignoring the breakpoint\n");
      return AE_OK;

   } else {

      panic("ACPI: unknown signal: %u\n", Function);
   }
}

ACPI_STATUS
AcpiOsEnterSleep(
    UINT8                   SleepState,
    UINT32                  RegaValue,
    UINT32                  RegbValue)
{
   printk("ACPI sleep: %u, 0x%x, 0x%x\n", SleepState, RegaValue, RegaValue);
   return AE_OK;
}

ACPI_PRINTF_LIKE (1)
void ACPI_INTERNAL_VAR_XFACE
AcpiOsPrintf(const char *Format, ...)
{
   va_list args;
   va_start(args, Format);
   vprintk(Format, args);
   va_end(args);
}

void
AcpiOsVprintf(
    const char              *Format,
    va_list                 Args)
{
   vprintk(Format, Args);
}

/* ACPI DEBUGGER funcs: keep not implemented for the moment */

void
AcpiOsRedirectOutput(void *Destination)
{
   printk("AcpiOsRedirectOutput ignored\n");
}

ACPI_STATUS
AcpiOsGetLine (
    char                    *Buffer,
    UINT32                  BufferLength,
    UINT32                  *BytesRead)
{
   NOT_IMPLEMENTED();
}

ACPI_STATUS
AcpiOsInitializeDebugger(void)
{
   /* do nothing */
   return AE_OK;
}

void
AcpiOsTerminateDebugger(void)
{
   /* do nothing */
}

ACPI_STATUS
AcpiOsWaitCommandReady(void)
{
   NOT_IMPLEMENTED();
}

ACPI_STATUS
AcpiOsNotifyCommandComplete(void)
{
   NOT_IMPLEMENTED();
}

void
AcpiOsTracePoint(
    ACPI_TRACE_EVENT_TYPE   Type,
    BOOLEAN                 Begin,
    UINT8                   *Aml,
    char                    *Pathname)
{
   /* do nothing */
}
