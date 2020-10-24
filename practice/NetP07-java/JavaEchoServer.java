// JavaEchoServer.java
 
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;
 
public class JavaEchoServer {
  public static void main(String[] args) {
    ServerSocket serverSocket = null;
    String serverPort = "30000";
    if (args.length==1)
    	serverPort = args[0];
    try {
      serverSocket = new ServerSocket(Integer.parseInt(serverPort));
      System.out.println(getTime() + " JavaEchoServer Running.");
    } catch (IOException e) {
      e.printStackTrace();
    } // try - catch
     
    while (true) {
      try {
        System.out.println(getTime() + " Waiting client connection..");
        Socket socket = serverSocket.accept();
        System.out.println(getTime() + " " + socket.getInetAddress() 
        				+ " connected.");
     
        OutputStream out = socket.getOutputStream();
        DataOutputStream dos = new DataOutputStream(out);
        InputStream in = socket.getInputStream();
        DataInputStream dis = new DataInputStream(in);
        
        while (true) {
        	String msg;
        	// client 로부터 읽고
        	//msg  = dis.readUTF();
        	//System.out.println(getTime() + " received " +  msg);
        	
        	byte[] b = new byte[128];	
        	int ret;
			ret = dis.read(b, 0, 128);
			msg = new String(b, "euc-kr");
			msg = msg.trim(); // 앞뒤 blank 제거
			System.out.println(getTime() + " received " +  ret + " " + msg);
			
     	
        	if (msg.contains("exit"))
        		break;
        	// client 에게 시간 붙여서 전송
        	msg = getTime() + " " + msg;	 
        	//dos.writeUTF(msg);
        	
        	// 128 byte로 고정시키기 위함
        	msg = String.format("%-128s", msg);
        	byte[] bb;
			bb = msg.getBytes("euc-kr"); // �ѱ� �ϼ��� �ڵ� ���
			//System.out.println("sending = " + msg.length());
			dos.write(bb,0, msg.length());
        }
        dis.close();
        dos.close();
        socket.close();
      } catch (IOException e) {
        e.printStackTrace();
      } // try - catch
    } // while
  } // main
   
  static String getTime() {
    SimpleDateFormat f = new SimpleDateFormat("[hh:mm:ss]");
    return f.format(new Date());
  } // getTime
}
