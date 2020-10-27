// JavaChatServer.java
// Java Chatting Server
import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Vector;
import java.awt.event.ActionEvent;
import javax.swing.SwingConstants;

public class JavaChatServer extends JFrame {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JPanel contentPane;
	JTextArea textArea;
	private JTextField txtPortNumber;

	private ServerSocket socket; // 서버소켓
	private Socket client_socket; // accept() 에서 생성된 client 소켓
	private Vector UserVec = new Vector(); // 연결된 사용자를 저장할 벡터
	private static final int BUF_LEN = 128; // Windows 처럼 BUF_LEN 을 정의

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					JavaChatServer frame = new JavaChatServer();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	public JavaChatServer() {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 338, 386);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);

		JScrollPane scrollPane = new JScrollPane();
		scrollPane.setBounds(12, 10, 300, 244);
		contentPane.add(scrollPane);

		textArea = new JTextArea();
		textArea.setEditable(false);
		scrollPane.setViewportView(textArea);

		JLabel lblNewLabel = new JLabel("Port Number");
		lblNewLabel.setBounds(12, 264, 87, 26);
		contentPane.add(lblNewLabel);

		txtPortNumber = new JTextField();
		txtPortNumber.setHorizontalAlignment(SwingConstants.CENTER);
		txtPortNumber.setText("30000");
		txtPortNumber.setBounds(111, 264, 199, 26);
		contentPane.add(txtPortNumber);
		txtPortNumber.setColumns(10);

		JButton btnServerStart = new JButton("Server Start");
		btnServerStart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					socket = new ServerSocket(Integer.parseInt(txtPortNumber.getText()));
				} catch (NumberFormatException | IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				AppendText("Chat Server Running..");
				btnServerStart.setText("Chat Server Running..");
				btnServerStart.setEnabled(false); // 서버를 더이상 실행시키지 못 하게 막는다
				txtPortNumber.setEnabled(false); // 더이상 포트번호 수정못 하게 막는다
				AcceptServer accept_server = new AcceptServer();
				accept_server.start();
			}
		});
		btnServerStart.setBounds(12, 300, 300, 35);
		contentPane.add(btnServerStart);
	}

	// 새로운 참가자 accept() 하고 user thread를 새로 생성한다.
	class AcceptServer extends Thread {
		@SuppressWarnings("unchecked")
		public void run() {
			while (true) { // 사용자 접속을 계속해서 받기 위해 while문
				try {
					AppendText("Waiting clients ...");
					client_socket = socket.accept(); // accept가 일어나기 전까지는 무한 대기중
					AppendText("새로운 참가자 from " + client_socket);
					// User 당 하나씩 Thread 생성
					UserService new_user = new UserService(client_socket);
					UserVec.add(new_user); // 새로운 참가자 배열에 추가
					AppendText("사용자 입장. 현재 참가자 수 " + UserVec.size());
					new_user.start(); // 만든 객체의 스레드 실행
				} catch (IOException e) {
					AppendText("!!!! accept 에러 발생... !!!!");
				}
			}
		}
	}

	public void AppendText(String str) {
		// textArea.append("사용자로부터 들어온 메세지 : " + str+"\n");
		textArea.append(str + "\n");
		textArea.setCaretPosition(textArea.getText().length());
	}

	// User 당 생성되는 Thread
	// Read One 에서 대기 -> Write All
	class UserService extends Thread {
		private InputStream is;
		private OutputStream os;
		private DataInputStream dis;
		private DataOutputStream dos;
		private Socket client_socket;
		private Vector user_vc;
		public String UserName = "";
		public String UserStatus = "O";

		public UserService(Socket client_socket) {
			// TODO Auto-generated constructor stub
			// 매개변수로 넘어온 자료 저장
			this.client_socket = client_socket;
			this.user_vc = UserVec;
			try {
				is = client_socket.getInputStream();
				dis = new DataInputStream(is);
				os = client_socket.getOutputStream();
				dos = new DataOutputStream(os);
				// line1 = dis.readUTF();
				// /login user1 ==> msg[0] msg[1]
				byte[] b = new byte[BUF_LEN];
				dis.read(b);
				String line1 = new String(b);
				String[] msg = line1.split(" ");
				UserName = msg[1].trim();
				
				AppendText("새로운 참가자 " + UserName + " 입장.");
				WriteOne("Welcome to Java chat server\n");
				WriteOne(UserName + "님 환영합니다.\n"); // 연결된 사용자에게 정상접속을 알림
				WriteAll("[" + UserName + "]님이 입장하였습니다.\n"); // 모든 참가자에게 입장 알림
			} catch (Exception e) {
				AppendText("userService error");
			}
		}

		// 모든 User들에게 방송. 각각의 UserService Thread의 WriteOne() 을 호출한다.
		public void WriteAll(String str) {
			for (int i = 0; i < user_vc.size(); i++) {
				UserService user = (UserService) user_vc.elementAt(i);
				if (user.UserStatus.equals("O")) {
					user.WriteOne(str);
				}
			}
		}

		// Windows 처럼 message 제외한 나머지 부분은 NULL 로 만들기 위한 함수
		public byte[] MakePacket(String msg) {
			byte[] packet = new byte[BUF_LEN];
			byte[] bb = null;
			int i;
			for (i = 0; i < BUF_LEN; i++)
				packet[i] = 0;
			try {
				bb = msg.getBytes("euc-kr");
			} catch (UnsupportedEncodingException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			for (i = 0; i < bb.length; i++)
				packet[i] = bb[i];
			return packet;
		}

		// UserService Thread가 담당하는 Client 에게 1:1 전송
		public void WriteOne(String msg) {
			try {
				// dos.writeUTF(msg);
				byte[] bb;
				bb = MakePacket(msg);
				dos.write(bb, 0, bb.length);
			} catch (IOException e) {
				AppendText("dos.write() error");
				try {
					dos.close();
					dis.close();
					client_socket.close();
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				UserVec.removeElement(this); // 에러가 난 현재 객체를 벡터에서 지운다
				AppendText("사용자 퇴장. 현재 참가자 수 " + UserVec.size());
			}
		}

		public void run() {
			while (true) { // 사용자 접속을 계속해서 받기 위해 while문
				try {
					// String msg = dis.readUTF();
					byte[] b = new byte[BUF_LEN];
					int ret;
					ret = dis.read(b);
					if (ret < 0) {
						AppendText("dis.read() < 0 error");
						try {
							dos.close();
							dis.close();
							client_socket.close();
							UserVec.removeElement(this); // 에러가 난 현재 객체를 벡터에서 지운다
							WriteAll("[" + UserName + "]님이 퇴장하였습니다.\n");
							AppendText("사용자 [" + UserName + "] 퇴장. 남은 참가자 수 " + UserVec.size());
							break;
						} catch (Exception ee) {
							break;
						} // catch문 끝
					}
					
					String msg = new String(b, "euc-kr");
					msg = msg.trim(); // 앞뒤 blank NULL, \n 모두 제거
					
					try {
						String[] args = msg.split(" ");
						
						if (args[1].equals("/exit")) {
							try {
								dos.close();
								dis.close();
								client_socket.close();
								AppendText(msg);
								UserVec.removeElement(this); // 에러가 난 현재 객체를 벡터에서 지운다
								WriteAll("[" + UserName + "]님이 퇴장하였습니다.\n");
								AppendText("사용자 [" + UserName + "] 퇴장. 현재 참가자 수 " + UserVec.size());
								break;
							} catch (Exception ee) {
								break;
							} // catch문 끝						
						}
						if (args[1].equals("/list")) {
							WriteOne("User List\nname\tstatus\n---------------------------------\n");
							for (int j = 0; j < UserVec.size(); j++) {
								UserService user = (UserService) user_vc.elementAt(j);
								String buf = String.format("%s\t%s\n", user.UserName, user.UserStatus);
								WriteOne(buf);
							}
							AppendText(msg);
							continue;
						}
						if (args[1].equals("/to")) {	// 귓속말
							for (int j = 0; j < UserVec.size(); j++) {
								String whisper = "";
								String buf = "";
								UserService user = (UserService) user_vc.elementAt(j);
								
								if (args[2].equals(user.UserName) && user.UserStatus.equals("O")) {
									whisper = msg;
									whisper = whisper.substring(args[0].length() + args[1].length() + args[2].length() + 3);
									buf = String.format("[귓속말] [%s] %s", UserName, whisper);
									user.WriteOne(buf);
								}
							}
							AppendText(msg);
							continue;
						}
						if (args[1].equals("/sleep")) {
							UserStatus = "S";
							AppendText(msg);
							continue;
						}
						if (args[1].equals("/wakeup")) { 
							UserStatus = "O";
							AppendText(msg);
							continue;
						}
						AppendText(msg); // server 화면에 출력
						WriteAll(msg + "\n"); // Write All
					} catch (ArrayIndexOutOfBoundsException e) { // sleep 상태에서 enter key치면 wakeup
						UserStatus = "O";
						AppendText(msg);
						continue;
					}
				} catch (IOException e) {
					AppendText("dis.read() error");
					try {
						dos.close();
						dis.close();
						client_socket.close();
						UserVec.removeElement(this); // 에러가 난 현재 객체를 벡터에서 지운다
						WriteAll("[" + UserName + "]님이 퇴장하였습니다.\n");
						AppendText("사용자 [" + UserName + "] 퇴장. 남은 참가자 수 " + UserVec.size());
						break;
					} catch (Exception ee) {
						break;
					} // catch문 끝
				} // 바깥 catch문끝
			} // while
		} // run
	}
}
