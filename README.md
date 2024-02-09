# LoopHPCs: loop-centric HPCs profiling .


# Introduction
LoopHPCs is a loop-centric HPCs profiling framework.
In the framework, we profile the loop structure of a running program with the help of two hardware mechanisms: LBR and PEBs.
LoopHPCs is suitable to analysis the malware which has distinct hot loop at run-time. The current version of LoopHPCs focus on binary unpacking. In the future, we plan to extend it to analyze other types of malware, such as ransomware and cryptojacking.



# Usage
To load compiled dirver of LoopHPCs into OS, the user can leaverage a driver load tool (e.g., DriverMonitor) or develop a custom driver load application.
The output information can by caputure by the tool of Dbgview.


        

