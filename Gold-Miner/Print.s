; Print.s
; Student names: Marzooq Shah and Shriya Vijay
; Last modification date: Apr 13, 2022
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
		
	PRESERVE8
		
;LOCAL VARIABLES

FP 		RN 	11
COUNT 	EQU 0
NUM 	EQU 4
counter EQU 0
NUM2		EQU 4
  

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
		
		


;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; R0=0,    then output "0"
; R0=3,    then output "3"
; R0=89,   then output "89"
; R0=123,  then output "123"
; R0=9999, then output "9999"
; R0=4294967295, then output "4294967295"
LCD_OutDec

	  PUSH {R4-R9, R11, LR}
	  SUB SP, #8
	  MOV FP, SP
	  STR R0, [FP, #NUM]
	  CMP R0, #0
	  BEQ ZERO
	  MOV R4, #0
	  STR R4, [FP, #COUNT]
	  MOV R5, #10
LOOP
	  LDR R4, [FP, #NUM]
	  CMP R4, #0
	  BEQ NEXT
	  UDIV R6, R4, R5
	  MUL R7, R6, R5
	  SUB R8, R4, R7
	  PUSH {R8}
	  STR R6, [FP, #NUM]
	  LDR R6, [FP, #COUNT]
	  ADD R6, R6, #1
	  STR R6, [FP, #COUNT]
	  B LOOP
	  
ZERO
	  PUSH{R0}
	  LDR R4, [FP, #COUNT]
	  ADD R4, #1
	  STR R4, [FP, #COUNT]
	  
NEXT
	  LDR R4, [FP, #COUNT]
	  CMP R4, #0
	  BEQ DONE
	  POP {R0}
	  ADD R0, #0x30
	  BL ST7735_OutChar 
	  SUB R4, #1
	  STR R4, [FP, #COUNT]
	  B NEXT

DONE
	  ADD SP, #8
	  POP {R4-R9, R11, LR}

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000"
;       R0=3,    then output "0.003"
;       R0=89,   then output "0.089"
;       R0=123,  then output "0.123"
;       R0=9999, then output "9.999"
;       R0>9999, then output "*.***"
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix

	 PUSH {R4-R9, R11, LR}
	 SUB SP, #8
	 MOV FP, SP
	 STR R0, [FP, #NUM2]
	 MOV R4, #0
	 STR R4, [FP, #counter]
	 MOV R5, #10
	 
	 ;If R0 > 9999 Edge case
	 MOV R4, #9999
	 SUBS R1, R0, R4
	 BHI Edge
	 
loopOut
	  LDR R3, [FP, #counter]
	  CMP R3, #5
	  BEQ Output
	  CMP R3, #3
	  BEQ Point
	  LDR R4, [FP, #NUM2]
	  UDIV R6, R4, R5
	  MUL R7, R6, R5
	  SUB R8, R4, R7
	  ADD R8, #0x30
	  PUSH {R8}
	  STR R6, [FP, #NUM2]
	  LDR R6, [FP, #counter]
	  ADD R6, R6, #1
	  STR R6, [FP, #counter]
	  B loopOut

Point
	 MOV R6, #0x2E
	 PUSH {R6}
	 ADD R3, #1
	 STR R3, [FP, #counter]
	 B loopOut

Edge
	 MOV R0, #0x2A
	 BL ST7735_OutChar
	 MOV R0, #0x2E
	 BL ST7735_OutChar
	 MOV R0, #0x2A
	 BL ST7735_OutChar
	 MOV R0, #0x2A
	 BL ST7735_OutChar
	 MOV R0, #0x2A
	 BL ST7735_OutChar
	 
	 B Finished

Output
	 LDR R4, [FP, #counter]
	 CMP R4, #0
	 BEQ Finished
	 POP {R0}
	 BL ST7735_OutChar 
	 SUBS R4, #1
	 STR R4, [FP, #counter]
	 B Output
	 
Finished
	 ADD SP, #8
	 POP {R4-R9, R11, LR}

     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
