; Luke Soldano, Xavier Agostini (C) 2016
; McGill University, ECSE 426, Lab One
; Assembly code to implement a Kalmann filter

	AREA fn, CODE, READONLY
	EXPORT Kalmanfilter_asm
Kalmanfilter_asm

; Filter takes four input paramaters
; 1. A pointer to the input data array
; 2. A pointer to the filtered data array
; 3. The length of the arrays
; 4. A pointer to the Kalmann filter struct/state

; For the function return value, registers R0/R1 & S0/S1 
; are used for integer and floating-point results respectively

; Register inputs (Input array, output array, array length, kalman struct address)

; Filter will hold its state as a quintuple (q,r,x,p,k) - all fp #'s
; Filter will load these values into registers (S4,S5,S6,S7,S8)

; Initialize q,r,x,p,k
; Load kalman state values into proper fp registers
; Check for overflowed inputs
	VLDR.f32 S4, [R3, #0] ; float q
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT
	
	VLDR.f32 S5, [R3, #4] ; float r 
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT
	
	VLDR.f32 S6, [R3, #8] ; float x
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT
	
	VLDR.f32 S7, [R3, #12] ; float p
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT

	VLDR.f32 S8, [R3, #16] ; float k
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT

; Initialize loop counter
	MOV R4, #0
	
; Start loop
LOOP

; Load input array value and check for overflowed input
	VLDR.f32 S9, [R0, #0] ; load proper index of input array to S9 (measurement value)
	VMRS APSR_nzcv, FPSCR
	BVS ERROR_OUT	
	
; Find values of p and k and make sure no overflow by additions
	VADD.f32 S7, S7, S4 ; p = p + q 
	VADD.f32 S10, S7, S5 ; p + r 

; Check for zero (not a number) in divisor
; Will also check for overflow - S10 won't be written to if overflow occurs in addition
	VCMP.f32 S10, #0
	VMRS APSR_nzcv, FPSCR
	BEQ ERROR_OUT

; Finish operation on k
	VDIV.f32 S8, S7, S10 ; k = p / (p + r)
	
; Find value of x
	VSUB.f32 S10, S9, S6 ; (measurement - x)
	VFMA.f32 S6, S8, S10 ; x + k * (measurement - x)

; Find value of p
	VFMS.f32 S7, S8, S7 ; p = (1 - k) * p = -pk + p

; Update output array and struct values
	VSTM R3, {S4-S8}
	VSTM R1, S6 
	
; Increment R0 & R1 addresses to return proper pointer for next iteration
	ADD R0, R0, #4
	ADD R1, R1, #4
	
; Determine whether or not loop should continue
	ADD R4, R4, #1
	CMP R4, R2
	BLT LOOP

; Load 0 into return register to indicate no error
	MOV R0, #0

; Return from branch
	BX LR
	
; Function called when overflow or other error detected
ERROR_OUT

; Load non-zero value into return register to indicate error
	MOV R0, #-1
	BX LR
	
	END
		
