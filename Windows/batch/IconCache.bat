@echo off
taskkill /f /im explorer.exe
attrib -h %LocalAppdata%\IconCache.db
del %LocalAppdata%\IconCache.db
start explorer.exe
@echo off
pause
exit