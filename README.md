# NetP04
- 로그인 프로토콜 구현
- 메뉴
  <ol>
  1. 소문자 → 대문자<br>
  2. 대문자 → 소문자<br>
  3. 소문자는 대문자로, 대문자는 소문자로<br>
  4. 종료
  </ol>
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/wecho_protocol.png" width="500" height="400">
<details>
  <summary>실행 결과</summary>
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/wecho_screenshot2.png" width="800" height="500">
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/wecho_screenshot1.png" width="800" height="500">
</details>

# NetP05
- Windows ↔ Windows/Linux 양방향 파일 전송
- 명령어 종류
  - get, put, quit, dir, ldir, !cmd
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_protocol.png" width="600" height="400">
<details>
  <summary>실행 결과</summary>
  <ul>
    <li>put</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_put.png" width="700" height="200">
    <li>get</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_get.png" width="700" height="200">
    <li>dir</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_dir.png" width="700" height="500">
    <li>ldir</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_ldir.png" width="700" height="200">
    <li>!ipconfig</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_cmd.png" width="700" height="500">
</details>

# NetP06
- /to, /sleep, /wakeup : ~~구현했는데 과제 변경으로 빠져버렸다..😭~~
- 1:1 채팅<br>
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/user-to-user-chatting-protocol.png" width="500" height="350">
- 파일 전송<br>
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_transfer_protocol.png" width="500" height="350">
<details>
  <summary>실행 결과</summary>
  <ul>
    <li>입/퇴장 알림</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/welcome_exit_message.png" width="500" height="250">
    <li>귓속말</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/whisper.png" width="500" height="400">
    <li>sleep/wakeup</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/sleep_wakeup_userlist.png" width="700" height="500">
    <li>1:1 채팅</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/user-to-user-chatting.png" width="700" height="500">
    <li>파일 전송</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/file_transfer.png" width="700" height="500">
</details>
  
# NetP07
- C로 구현했던 NetP06에서의 기능(입퇴장 알림, /exit, /list, /to, /sleep, /wakeup)들을 Java로 구현하기
<details>
  <summary>실행 결과</summary>
  <ul>
    <li>서버</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/java-chat-server.png" width="500" height="150">
    <li>클라이언트</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/java-chat-client.png" width="500" height="150">
</details>

# NetP-midterm
- 레스토랑 POS Client/Server 프로그램
- Client는 주문 모드(메뉴와 수량 정보)와 주방 모드(완료된 메뉴와 수량 정보)로 동작
- Server는 주문 현황을 출력하고 Client에 전송<br>
  <img src="https://github.com/coding-Benny/network-programming/blob/master/images/restaurant-pos-protocol.png" width="500" height="350">
<details>
  <summary>실행 결과</summary>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/restaurant-pos.png" width="600" height="500">
</details>
  
# NetP09
- 도전 과제
  - 본인 Message는 카톡처럼 우측에 나오기
  - 사용자 List 화면 추가하기
  - /sleep, /wakeup 대신 GUI 버튼 사용하기(JComboBox, JCheckBox 등)
  - 이모티콘 추가하기
<details>
  <summary>실행 결과</summary>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/challenge-server.PNG" width="200" height="300">
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/challenge-client.PNG" width="1000" height="400">
</details>

# NetP10
- 그림판 공유
- 기능 추가
  - Dot 대신 Line 그리기
  - Color 선택하기
  - 지우개 기능
<details>
  <summary>실행 결과</summary>
  <ul>
    <li>그림판 공유</li>
     <img src="https://github.com/coding-Benny/network-programming/blob/master/images/share-painting.png" width="1200">
    <li>기능 추가</li>
      <img src="https://github.com/coding-Benny/network-programming/blob/master/images/advanced-painting.png" width="1200">
  </ul>
</details>
