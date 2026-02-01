$portName = "COM7"

$baudRate = 115200
$port = New-Object System.IO.Ports.SerialPort $portName, $baudRate, None, 8, one
$port.DtrEnable = $true
$port.RtsEnable = $true

$port.Open()
try {
    Write-Host "$portName を受信中... 終了するには Ctrl + C を押してください。" -ForegroundColor Cyan

    while ($true) {
        if ($port.BytesToRead -gt 0) {
            $data = $port.ReadExisting()
            Write-Host $data -NoNewline
        }
        Start-Sleep -Milliseconds 10
    }
}
catch {
    Write-Error "エラーが発生しました: $_"
}
finally {
    if ($null -ne $port -and $port.IsOpen) {
        $port.Close()
        Write-Host "`n$portName を閉じました。スクリプトを終了します。" -ForegroundColor Yellow
    }
}
