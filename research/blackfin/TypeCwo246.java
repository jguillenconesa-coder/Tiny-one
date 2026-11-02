// SPDX-License-Identifier: AGPL-3.0-only
// Analyst-owned ABI metadata. Generated pseudocode remains private.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.nio.file.*;
import java.util.*;
public class TypeCwo246 extends GhidraScript {
 public void run() throws Exception {
  String[] args=getScriptArgs();DataType ip=new PointerDataType(IntegerDataType.dataType),sp=new PointerDataType(ShortDataType.dataType),vp=new PointerDataType(VoidDataType.dataType),i=IntegerDataType.dataType;
  Object[][] entries={{"01944528",ip,sp},{"019445a8",sp,sp},{"01944660",ip,sp},{"0194435c",ip,sp,sp,ip},{"0194444e",sp,sp},{"01943b2e",sp,sp,sp,sp},{"01943744",ip,sp,i,i,sp,i},{"01943bea",ip,sp,ip,sp},{"01943e0a",ip,ip,ip,ip,ip},{"019446e0",vp,ip,ip,sp,sp,sp},{"01944480",ip,sp,sp},{"0194480c",vp,sp,sp}};
  var instructions=currentProgram.getListing().getInstructions(toAddr("01943700"),true);
  while(instructions.hasNext()){var instruction=instructions.next();if(instruction.getAddress().compareTo(toAddr("01946400"))>=0)break;var previous=getInstructionBefore(instruction.getAddress());if(previous!=null&&previous.getMnemonicString().equals("UNLINK")&&instruction.getMnemonicString().equals("JUMP"))instruction.setFlowOverride(FlowOverride.RETURN);}
  for(Object[] e:entries){var address=toAddr((String)e[0]);disassemble(address);Function f=getFunctionAt(address);if(f==null)f=createFunction(address,"cwo_"+e[0]);List<Parameter> p=new ArrayList<>();for(int n=1;n<e.length;++n){var storage=n<=3?new VariableStorage(currentProgram,currentProgram.getRegister("R"+(n-1))):new VariableStorage(currentProgram,12+4*(n-4),4);p.add(new ParameterImpl("p"+n,(DataType)e[n],storage,currentProgram));}f.setReturnType(VoidDataType.dataType,SourceType.USER_DEFINED);f.replaceParameters(p,Function.FunctionUpdateType.CUSTOM_STORAGE,true,SourceType.USER_DEFINED);}
  DecompInterface decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
  try{var f=getFunctionAt(toAddr("0194480c"));var result=decompiler.decompileFunction(f,90,monitor);if(!result.decompileCompleted())throw new Exception(result.getErrorMessage());Files.writeString(Path.of(args[0],"cwo-typed.c"),result.getDecompiledFunction().getC());println("CWO typed decompilation complete");}finally{decompiler.dispose();}
 }
}
