# Hardware & Embedded Systems Projects

Xin chào, mình là **Trần Đức Lâm**, sinh viên ngành **Điện tử – Viễn thông**.

Repository này tổng hợp các dự án mình đã thực hiện trong quá trình học tập và phát triển kỹ năng về:

- Embedded Systems
- PCB & Schematic Design
- AIoT
- Computer Vision
- Sensor & Actuator Integration
- Robotics & Control
- Hardware–Software Co-design

Phần lớn các dự án được triển khai từ quá trình **thiết kế mạch nguyên lý, thiết kế PCB, lập trình bộ điều khiển, tích hợp cảm biến – cơ cấu chấp hành đến kiểm thử hệ thống thực tế**.

Mình hiện đặc biệt quan tâm đến **Embedded Systems, AIoT, Digital Design, FPGA/SoC và AI Hardware**, với mục tiêu tiếp tục phát triển theo hướng kết hợp giữa phần cứng và phần mềm.

---

## Projects

### 1. [AI-Based Tomato Sorting Conveyor](./PhanLoaiCaChua/)

Hệ thống băng tải tự động phân loại cà chua ứng dụng **Computer Vision và YOLOv8**.

**Các thành phần chính:**

- Webcam thu nhận hình ảnh
- YOLOv8 xử lý và phân loại cà chua
- Giao tiếp UART giữa chương trình AI và bộ điều khiển
- Cảm biến vật cản
- Load cell đo khối lượng
- Servo điều khiển cơ cấu phân loại
- PCB điều khiển hệ thống

**Chức năng chính:**

- Nhận diện cà chua xanh, chín và hỏng
- Đo khối lượng sản phẩm
- Điều khiển cơ cấu phân loại tự động
- Kết hợp xử lý AI trên máy tính với hệ thống nhúng

---

### 2. [IoT Smart Home System](./NhaThongMinh/)

Hệ thống nhà thông minh tích hợp nhiều cảm biến, thiết bị điều khiển và khả năng giám sát từ xa.

**Các thành phần chính:**

- MQ-2 – cảm biến khói/khí gas
- DHT11 – cảm biến nhiệt độ, độ ẩm
- AS608 – cảm biến vân tay
- LCD
- LED
- Servo
- Quạt
- ESP32 / vi điều khiển
- Blynk IoT
- GSM/SIM

**Chức năng chính:**

- Giám sát các thông số môi trường
- Xác thực bằng vân tay
- Điều khiển thiết bị từ xa qua Blynk
- Gửi cảnh báo về điện thoại
- Tích hợp cảm biến và cơ cấu chấp hành trên PCB tự thiết kế

---

### 3. [Line Following Robot](./RobotDoLine/)

Robot tự động dò và bám theo đường sử dụng cảm biến và thuật toán điều khiển.

**Các thành phần chính:**

- Cảm biến dò line
- Vi điều khiển
- Motor driver
- Động cơ DC
- PCB điều khiển

**Thuật toán:**

- Đọc và xử lý tín hiệu từ cảm biến
- Xác định sai lệch so với đường đi
- Sử dụng **PID Control** để điều chỉnh tốc độ động cơ
- Giúp robot di chuyển ổn định theo quỹ đạo

---

### 4. [Three-Phase Motor Control System](./MotorControl/)

Hệ thống điều khiển **đóng/ngắt và đảo chiều động cơ 3 pha** thông qua relay và tín hiệu cảm biến.

**Các thành phần chính:**

- Relay điều khiển tải
- Cảm biến
- Bộ điều khiển
- Mạch giao tiếp điều khiển
- PCB

**Chức năng chính:**

- Đóng/ngắt động cơ
- Điều khiển đảo chiều
- Nhận tín hiệu từ cảm biến
- Điều khiển relay theo logic hệ thống

---

### 5. [Obstacle Warning & GPS Tracking Device](./ThietBiCanhBao/)

Thiết bị hỗ trợ phát hiện vật cản và gửi thông tin vị trí tới điện thoại.

**Các thành phần chính:**

- Cảm biến siêu âm
- Động cơ rung
- NEO GPS
- GSM/SIM
- Vi điều khiển
- PCB

**Chức năng chính:**

- Phát hiện vật cản phía trước
- Cảnh báo bằng rung
- Xác định vị trí GPS
- Gửi thông tin vị trí về điện thoại qua mạng di động

---

### 6. [Drone](./Drone/)

Dự án nghiên cứu và thiết kế hệ thống điều khiển cho drone, tập trung vào **embedded hardware, sensor integration và motor control**.

Repository chứa các tài liệu liên quan đến:

- Schematic
- PCB Design
- Bộ điều khiển
- Cảm biến
- Driver động cơ
- Firmware / source code
- Tài liệu và hình ảnh thiết kế

---

### 7. [Automatic Stair LED System](./LedCauThang/)

Hệ thống điều khiển LED cầu thang tự động.

Dự án tập trung vào:

- Thiết kế mạch điều khiển
- Xử lý tín hiệu cảm biến
- Điều khiển LED
- PCB Design
- Embedded programming

Chi tiết thiết kế phần cứng và source code được lưu trong thư mục dự án.

---

### 8. [Potato Sorting System](./PhanLoaiKhoaiTay/)

Hệ thống tự động phục vụ quá trình **phân loại khoai tây**.

Dự án bao gồm các nội dung liên quan đến:

- Thiết kế hệ thống
- Cảm biến
- Bộ điều khiển
- Cơ cấu chấp hành
- PCB
- Source code

Chi tiết schematic, PCB và chương trình điều khiển được lưu trong thư mục dự án.

---

## Repository Structure

```text
hardware-embedded-projects/
│
├── Drone/
│
├── LedCauThang/
│
├── MotorControl/
│
├── NhaThongMinh/
│
├── PhanLoaiCaChua/
│
├── PhanLoaiKhoaiTay/
│
├── RobotDoLine/
│
└── ThietBiCanhBao/
