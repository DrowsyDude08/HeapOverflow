# Скрипт настройки кодировки UTF-8 для PowerShell
# Используется для корректного отображения русского текста

Write-Host "Настройка кодировки UTF-8..." -ForegroundColor Green

# Устанавливаем кодировку UTF-8 для текущей сессии
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

# Устанавливаем кодовую страницу UTF-8
chcp 65001 | Out-Null

Write-Host "Кодировка UTF-8 установлена!" -ForegroundColor Green
Write-Host "Теперь можно запускать exploit.py" -ForegroundColor Yellow

