// JavaEchoClient.java

import java.util.Scanner;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ConnectException;
import java.net.Socket;

public class JavaEchoClient {
	public static void main(String[] args) {
		try {
			String serverIP = "127.0.0.1"; // 127.0.0.1 & localhost
			String serverPort = "30000";
			if (args.length == 2) {
				serverIP = args[0];
				serverPort = args[1];
			}

			System.out.println("Connecting " + serverIP + " " + serverPort);
			Socket socket = new Socket(serverIP, Integer.parseInt(serverPort));
			System.out.println("Connected.");

			OutputStream out = socket.getOutputStream();
	        DataOutputStream dos = new DataOutputStream(out);
	        InputStream in = socket.getInputStream();
	        DataInputStream dis = new DataInputStream(in);

        	Scanner sc = new Scanner(System.in);
        	String buf;
	        while (true) {
	           	// keyboard에서 읽고
	        	System.out.print("Input string : ");
	        	buf = sc.nextLine();
	        	dos.writeUTF(buf);

	        	if (buf.contains("exit"))
	        		break;

	        	// server에서 수신하고
	        	String msg;
	        	msg = dis.readUTF();

				// 화면에 출력
	        	System.out.println(msg);
	        }
	        dos.close();
			dis.close();
			sc.close();
			socket.close();
		} catch (ConnectException ce) {
			ce.printStackTrace();
		} catch (IOException ie) {
			ie.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		} // try - catch
	} // main
} // 
