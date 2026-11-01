// SPDX-License-Identifier: AGPL-3.0-only
// Analyst-owned metadata only. All pseudocode stays in the private output.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FlowOverride;
import java.nio.file.*;
public class DecompileBoot246 extends GhidraScript {
 public void run() throws Exception {
  String[] args=getScriptArgs();
  for(int i=1;i<args.length;i++)disassemble(toAddr(args[i]));
  // These observed init-stage epilogues restore P0 from the saved return slot,
  // UNLINK, then JUMP(P0). Mark returns without changing executable bytes.
  if(currentProgram.getMemory().getBlock(".init_stage")!=null){
   for(String value:new String[]{"ffa00190","ffa00572","ffa1007c","ffa10112","ffa10124"}){
    var address=toAddr(value);var instruction=getInstructionAt(address);
    var previous=getInstructionBefore(address);
    if(instruction==null||previous==null||!instruction.getMnemonicString().equals("JUMP")||!previous.getMnemonicString().equals("UNLINK"))
     throw new Exception("Return annotation no longer matches observed epilogue "+value);
    instruction.setFlowOverride(FlowOverride.RETURN);
   }
  }
  DecompInterface decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
  try{
   for(int i=1;i<args.length;i++){
    var address=toAddr(args[i]);Function function=getFunctionAt(address);
    if(function==null)function=createFunction(address,"target_"+args[i]);
    if(function==null)throw new Exception("Cannot create function "+args[i]);
    DecompileResults result=decompiler.decompileFunction(function,90,monitor);
    if(!result.decompileCompleted())throw new Exception(result.getErrorMessage());
    String code=result.getDecompiledFunction().getC();
    Files.writeString(Path.of(args[0],args[i]+".c"),code);
    println("DECOMPILE_COMPLETED "+args[i]+" bytes="+code.length());
   }
  }finally{decompiler.dispose();}
 }
}
