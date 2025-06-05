🚧 Hệ thống phát hiện sự kiện và gửi vị trí GPS đến Google Sheet
📌 Mục tiêu
Xây dựng một hệ thống nhúng sử dụng ESP32, SIM module, và WiFi để:

Phát hiện sự kiện (như rung động hoặc va chạm) từ cảm biến.

Đọc vị trí GPS từ SIM module.

Chuyển tọa độ GPS thành địa chỉ cụ thể bằng API Nominatim (OpenStreetMap).

⚙️ Phần cứng sử dụng
ESP32

SIM7600/SIM800/SIM868 module có hỗ trợ GPS

Cảm biến rung hoặc công tắc từ (kết nối GPIO)

Dây kết nối, nguồn 3.7–4.2V cho SIM module

🧠 Nguyên lý hoạt động
ESP32 bật nguồn cho SIM module.

Kết nối WiFi để gửi dữ liệu đến Google Sheets.

Mở GPS và đọc dữ liệu từ module SIM qua UART.

Khi cảm biến phát hiện sự kiện (giá trị HIGH), thực hiện:

Lấy tọa độ GPS bằng lệnh AT+QGPSLOC=0

Chuyển đổi tọa độ thành định dạng thập phân

Gửi truy vấn đến API Nominatim để lấy địa chỉ cụ thể

Gửi địa chỉ này đến Google Sheet bằng HTTP GET

🛰️ Tính năng nổi bật
✅ Tích hợp WiFi và SIM module linh hoạt

✅ Lấy địa chỉ thật thay vì chỉ tọa độ GPS

✅ Gửi dữ liệu lên Google Sheet qua Google Apps Script

✅ Thích hợp cho hệ thống giám sát, phát hiện sự cố, cảnh báo môi trường

📦 Thư viện sử dụng
WiFi.h – để kết nối WiFi

HTTPClient.h – gửi HTTP GET

ArduinoJson.h – parse JSON từ OpenStreetMap API

HardwareSerial – giao tiếp UART với module SIM

🔗 API sử dụng
Google Apps Script Web App: nhận địa chỉ và ghi vào Google Sheets

Nominatim Reverse Geocoding API: https://nominatim.openstreetmap.org/reverse

📄 Cấu trúc file chính
main.ino: toàn bộ mã điều khiển ESP32

Bao gồm: bật SIM, đọc GPS, kết nối WiFi, gửi HTTP, xử lý JSON

📝 Yêu cầu
Tài khoản Google để tạo Google Apps Script nhận dữ liệu

ESP32 Dev Module

Cảm biến phù hợp và nguồn ổn định cho SIM

