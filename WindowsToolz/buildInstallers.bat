@echo off
candle installerMSI.wxs -ext WixUIExtension -ext WixUtilExtension
light installerMSI.wixobj -ext WixUIExtension -ext WixUtilExtension -out WindowsToolz.msi
candle installerEXE.wxs -ext WixBalExtension -ext WixUIExtension
light installerEXE.wixobj -ext WixBalExtension -ext WixUIExtension -out WindowsToolzInstaller.exe
pause