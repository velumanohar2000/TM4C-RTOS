	.def setPSP
	.def setControlRegister
	.def getPSP
	.def getMSP
	.def dividebyzero
	.def getR0
	.def getRegisters
	.def popFromPsp
	.def writeToLr
	.def unreadyPush


.thumb
.const
.text

setPSP:
	MSR PSP, R0
	ISB
	BX LR

setControlRegister:
	MRS R1, CONTROL
	AND R1, R1, #0
	ORR R1, R1, R0
	MSR CONTROL, R1
	ISB
	BX LR



getPSP:
	MRS R0, PSP
	BX LR

getMSP:
	MRS R0, MSP
	BX LR

getR0:
	BX LR

getRegisters:
	ADD R0, R0, #4
	STR R1, [R0]
	ADD R0, R0, #4
	STR R2, [R0]
	ADD R0, R0, #4
	STR R3, [R0]
	ADD R0, R0, #4
	STR R12, [R0]
	ADD R0, R0, #4
	MOV R1, PC
	STR R1, [R0]
	ADD R0, R0, #4
	STR LR, [R0]
	BX LR


dividebyzero:
	MOV R1, #0
	MOV R2, #10
	UDIV R0, R2, R1
	BX LR


popFromPsp:
	MRS R0, PSP
	LDMIA R0!, {R4-R11, LR} ;load register from values on psp
	MSR PSP, R0
	BX LR

writeToLr:
	MOV LR, R0
	BX LR


;R0 xPSR value
;R1 New Function Address
;R2 exception Return
unreadyPush:
	MOV R10, R2
	MRS R2, PSP

	STR R0, [R2], #-4   ;xPSR
	STR R1, [R2], #-4	;PC

	MOV R3, #0
	STR R3, [R2], #-4  	;LR
	STR R3, [R2], #-4  	;R12
	STR R3, [R2], #-4	;R3
	STR R3, [R2], #-4	;R2
	STR R3, [R2], #-4	;R1
	STR R3, [R2]		;R0
	MSR PSP, R2

	BX LR



