;Umut Kalelioglu - 20190808016
;Arjin Kavalci   - 20190808050
#Persistent 

; Set start and finish serial number
startserialnumber := 000000
finishserialnumber:= 000100
loopcount:= finishserialnumber-startserialnumber+1

;Create folder with group id
run cmd.exe /c "mkdir Group10",hide
Sleep, 500

Run, Ford.exe
WinWaitActive, ahk_exe Ford.exe

Loop %loopcount% {

     WinActivate, ahk_exe Ford.exe
     Sleep, 500 

     ;Assign corners of Ford.exe
     WinGetPos, WinX, WinY, WinWidth, WinHeight, A
     WinXEnd :=WinX+WinWidth
     WinYEnd :=WinY+WinHeight

     WinXForFive := (WinWidth*0.75)
     WinYForFive := (WinHeight*0.9)
     WinXForClear := (WinWidth*0.8)
     WinYForClear := (WinHeight*0.6)

     ;Enter the serial number
     SendInput,%startserialnumber%

     ;Press "button number 5" (adjust the key if needed)
     Click %WinXForFive%, %WinYForFive%

     ;Execute the screenshot.exe with corners and path options
     run screenshot-cmd.exe -rc %WinX% %WinY% %WinXEnd% %WinYEnd% -o Group10\%startSerialNumber%.png
     Sleep,500
     WinActivate, ahk_exe Ford.exe
     Sleep, 500 

     ;Press "Clear Serial Code"
     Click %WinXForClear%, %WinYForClear%
     startserialnumber++
     startserialnumber:= Format("{:06}", startserialnumber)
}

return