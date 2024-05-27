set input=%1%

set signtool="C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\signtool.exe"

@REM 指纹签名方式，需要在弹窗中输入证书密码
%signtool% sign /tr "http://timestamp.digicert.com" /td sha256 /fd sha256 /sha1 "******" %1%

@REM pfx 文件签名方式
@REM %signtool% sign /f "C:\code\certificate\digicert.pfx" /p password /fd SHA256 /tr "http://timestamp.digicert.com/scripts/timestamp.dll" /td SHA256 %1%
