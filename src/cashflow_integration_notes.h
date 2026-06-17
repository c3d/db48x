#pragma once

/*
DB48X integration notes for HP 17bii-style CFLO support.

1. Add the following commands to the finance command table:

   CFLO      - enter cashflow editor/menu
   CFIN      - append amount to active list
   CFINS     - insert amount before current item
   CFDEL     - delete current item
   CFNAME    - save active list under a name
   CFGET     - recall named list
   CFTIMES   - toggle prompting for repetition count
   NPV       - compute NPV from active cashflow list and I%
   IRR       - compute IRR from active cashflow list

2. Add a FinanceMenu entry modeled after existing TVM / AMORT menu blocks.

3. Maintain one finance-global CashFlowState instance, e.g.:

   namespace db48x::finance {
       extern CashFlowState g_cashflows;
   }

4. Prompt workflow should mimic HP 17bii behavior:

   - first amount is CF0
   - subsequent entries are CF1..n
   - if times prompting is enabled, ask for repetition count after amount entry
   - display count as Nj when count > 1

5. Menu behavior mapping:

   HP 17bii-ish          DB48X action
   ----------------------------------------------
   INPUT                 CFIN command
   INSR                  CFINS command
   DELET                 CFDEL command
   NAME                  CFNAME command
   GET                   CFGET command
   #TIMES                CFTIMES command/toggle
   CALC->NPV             NPV command
   CALC->IRR             IRR command

6. Serialization:

   Save 'saved' named lists in whatever persistent finance-state store DB48X
   already uses for TVM / AMORT state.

7. RPL exposure:

   To stay idiomatic with DB48X, consider exposing programmatic forms:

     { -20000 5000 7000 9000 } 10 NPV
     { -20000 {5000 2} {9000 3} } IRR

   The interactive finance menu can internally build the same CashFlowList.

8. Validation and UX:

   - reject count < 1
   - reject rates <= -100%
   - IRR requires at least one positive and one negative flow
   - show 'No IRR' or 'IRR did not converge' on failure
   - document that I% is periodic, like the HP 17bii

9. Suggested test cases:

   a) CF0=-100, CF1=60, CF2=60, I%=10 => NPV ≈ 4.132231
   b) CF0=-100, CF1=60, CF2=60 => IRR ≈ 13.066239%
   c) CF0=0, CF1=100x3, CF2=200x2, CF3=300x4, I%=10
   d) all-positive flows => IRR failure
   e) empty list => NPV/IRR error

10. Registration sketch:

   REGISTER_COMMAND(CFIN,   finance_cfin_command);
   REGISTER_COMMAND(CFINS,  finance_cfins_command);
   REGISTER_COMMAND(CFDEL,  finance_cfdel_command);
   REGISTER_COMMAND(CFNAME, finance_cfname_command);
   REGISTER_COMMAND(CFGET,  finance_cfget_command);
   REGISTER_COMMAND(CFTIMES, finance_cftimes_command);
   REGISTER_COMMAND(NPV,    finance_npv_command);
   REGISTER_COMMAND(IRR,    finance_irr_command);

11. UI/editor sketch:

   finance_cfin_command():
      prompt amount
      if g_cashflows.active.times_prompting: prompt count
      append entry
      redraw cashflow list

   finance_npv_command():
      prompt I%
      compute cmd_cflo_npv(g_cashflows, rate)
      show numeric result

   finance_irr_command():
      compute cmd_cflo_irr(g_cashflows)
      show numeric result or error
*/
