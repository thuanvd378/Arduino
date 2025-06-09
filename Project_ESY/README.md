# Dự án mở khóa vân tay 
Đây là repository cho dự án mở khóa sử dụng cảm biến vân tay AS608 với vi điều khiển Arduino R3.
## Nội dung Repository
1 . Hình ảnh sản phẩm

![Ảnh sản phẩm](https://i.postimg.cc/m2CCKJrW/Snapshot-1.png)

Dự án sử dụng cảm biến vân tay nhằm mang đến giải pháp bảo mật nâng cao hơn, tiện lợi hơn

![Ảnh sản phẩm](https://i.postimg.cc/sXF8w672/z6633986529070-66da47c504b5d1d282da2b937a125328.jpg)

Một góc nhìn khác từ trên xuống của sản phẩm
2 . Phần mềm ghi nhận thời gian ấn vân tay
Chương trình C# Winform Application code trên môi trường của Visual Studio, nhận lệnh được gửi thông qua COM kết nối 

Bài tập và ví dụ thực hành:
Các bài toán thực tế kèm hướng dẫn chi tiết.

## Yêu cầu
Lập trình vi điều khiển : Arduino IDE phiên bản mới nhất

Trình biên dịch phần mềm : Visual Studio 2019 hoặc mới hơn

Hệ điều hành: Có thể chạy trên Windows, macOS và Linux.

## Hướng dẫn Sử dụng
1 . Clone repository:
```bash
git clone https://github.com/your_username/your_repository.git
cd your_repository
```
2. Nạp code Arduino :

Nạp code (file .ino) cho vi điều khiển Arduino R3 bằng Arduino R3. Lưu ý chọn đúng cổng COM

```bash
Project_ESY.ino
```
3. Mở Visual Studio :

Mở file project của phần mềm, nhấn Start và chọn đúng cổng COM đã kết nối với vi điều khiển trước đó. Lưu ý tắt chương trình Arduino IDE trước khi chạy.

```bash
Project_ESY.sln
```
## Đóng góp
Nếu bạn muốn đóng góp cho dự án, vui lòng:

1. Fork repository.

2. Tạo branch mới cho tính năng hoặc sửa lỗi: git checkout -b feature/your-feature-name.

3. Commit thay đổi: git commit -m "Thêm tính năng ..." và push lên branch.

4. Tạo Pull Request.
