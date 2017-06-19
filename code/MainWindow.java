import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Label;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Text;

import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;

public class MainWindow {

	protected static Shell shell;
	private static GridLayout gl_shell;

	private Text text;
	private List list;
	private Button btnSearch;
	private Button btnDownload;

	public final static String sFolderName = "SharedFolder";
	public final static String dFolderName = "DownloadFolder";

	private static ServerSocket clientSocket = null;
	private static int clientPort = -1;
	private static String clientIP = null;

	private final static String serverName = "localhost";
	private final static int serverPort = 44444;
	private static Socket serverSocket = null;
	private static OutputStream output = null;
	private static InputStream input = null;

	/**
	 * Launch the application.
	 * 
	 * @param args
	 */
	public static void main(String[] args) {
//		System.out.println("<main>");

		onShutDown();

		MainWindow window = new MainWindow();

		new Thread(new Runnable() {
			public void run() {
				window.open();
			}
		}).start();

		new Thread(new Runnable() {
			public void run() {
				try {
					openServerSocket();
					connectToServer();
				} catch (UnknownHostException e) {
					System.out.println("UnknownHostException: " + e.getMessage());
				} catch (IOException e) {
					System.out.println("IOException: " + e.getMessage());
				}
			}
		}).start();

//		System.out.println("</main>");
	}

	/*
	 * open a Socket for incoming connections from other clients
	 */
	private static void openServerSocket() throws IOException {
//		System.out.println("<openServerSocket>");

		clientSocket = new ServerSocket(0);
		clientPort = clientSocket.getLocalPort();
		clientIP = InetAddress.getLocalHost().getHostAddress();

//		System.out.println("</openServerSocket>");
	}

	/*
	 * Run when the program exits
	 */
	private static void onShutDown() {
//		System.out.println("<onShutDown>");

		Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
			public void run() {
				System.out.println("In shutdown hook");
				try {
					if (clientSocket != null && !clientSocket.isClosed())
						clientSocket.close();
				} catch (Exception e) {
					System.out.println(e.getMessage());
				}
				disconnectFromServer();
			}

			private void disconnectFromServer() {
				try {
					if (serverSocket != null) {
						if (output != null) {
							output.write("BYE\n".getBytes());
							output.close();
						}
						if (input != null) {
							input.close();
						}
						serverSocket.close();
					}
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}, "Shutdown-thread"));

//		System.out.println("</onShutDown>");
	}

	/*
	 * Connecting to the global FailMail Server and initial actions after the
	 * connection successfully established
	 */
	private static void connectToServer() throws UnknownHostException, IOException {
//		System.out.println("<connectToServer>");

		serverSocket = new Socket(serverName, serverPort);
		output = serverSocket.getOutputStream();
		input = serverSocket.getInputStream();

		while (true) {
			String s = "HELLO\n";
			output.write(s.getBytes());
			
			s = new BufferedReader(new InputStreamReader(input)).readLine();
			System.out.println("FROM SERVER: " + s);

			if (s.equals("HI")) {
				break;
			}
		}


		String sharedFiles = getSharedFilesList();
		output.write((sharedFiles + "\n").getBytes());

		new Thread(new Client(clientSocket)).start();

//		System.out.println("</connectToServer>");
	}

	private static String getSharedFilesList() {
//		System.out.println("<getSharedFilesList/>");

		try {
			File sharedFolder = createDirectory(sFolderName);
			String list = "";
			
			for (File file : sharedFolder.listFiles()) {
				if (file.isFile()) {
					list += "<" + file.getName() + ", "
							+ getFileExtension(file) + ", "
							+ file.length() + ", "
							+ new SimpleDateFormat("dd/MM/yyyy").format(new Date(file.lastModified())) + ", "
							+ clientIP + ", "
							+ clientPort + ">\n";
				}
			}
			
			return list;
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}
		return null;
	}

	private static String getFileExtension(File file) {
//		System.out.println("<getFileExtension/>");

		String fileName = file.getName();

		if (fileName.lastIndexOf(".") != -1 && fileName.lastIndexOf(".") != 0)
			return fileName.substring(fileName.lastIndexOf(".") + 1);
		else
			return "";
	}

	public static File createDirectory(String directoryPath) throws IOException {
//		System.out.println("<createDirectory/>");

		File dir = new File(directoryPath);
		if (dir.exists()) {
			return dir;
		}
		if (dir.mkdirs()) {
			return dir;
		}
		throw new IOException("Failed to create directory '" + dir.getAbsolutePath() + "' for an unknown reason.");
	}

	/**
	 * Open the window.
	 */
	public void open() {
//		System.out.println("<open>");

		Display display = Display.getDefault();
		createContents();
		shell.open();
		shell.layout();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}

//		System.out.println("</open>");
	}

	/**
	 * Create contents of the window.
	 */
	protected void createContents() {
//		System.out.println("<createContents>");

		shell = new Shell();
		shell.addListener(SWT.Close, new Listener(){

			@Override
			public void handleEvent(Event arg0) {
				System.exit(0);
				
			}});
		shell.setSize(660, 380);
		shell.setText("SWT Application");
		gl_shell = new GridLayout(14, false);
		gl_shell.marginRight = 20;
		shell.setLayout(gl_shell);
		// new Label(shell, SWT.NONE);

		text = new Text(shell, SWT.BORDER);
		text.setToolTipText("Enter filename ...");
		GridData gd_text = new GridData(SWT.FILL, SWT.CENTER, true, false, 12, 1);
		gd_text.widthHint = 513;
		text.setLayoutData(gd_text);

		btnSearch = new Button(shell, SWT.NONE);
		btnSearch.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseUp(MouseEvent e) {
				searchFile();
			}
		});
		GridData gd_btnSearch = new GridData(SWT.CENTER, SWT.CENTER, false, false, 1, 1);
		gd_btnSearch.widthHint = 60;
		btnSearch.setLayoutData(gd_btnSearch);
		btnSearch.setText("Search");
		new Label(shell, SWT.NONE);

		list = new List(shell, SWT.BORDER);
		GridData gd_list = new GridData(SWT.FILL, SWT.FILL, false, false, 12, 1);
		gd_list.heightHint = 269;
		list.setLayoutData(gd_list);

		btnDownload = new Button(shell, SWT.NONE);
		btnDownload.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseUp(MouseEvent e) {
				downloadFile();
			}
		});
		btnDownload.setText("Download");
		new Label(shell, SWT.NONE);

//		System.out.println("</createContents>");
	}

	protected void downloadFile() {
//		System.out.println("<downloadFile>");

		String[] selected = list.getSelection();
		if (selected.length > 0)
			new Thread(new DownloadRunnable(selected)).start();

//		System.out.println("</downloadFile>");
	}

	protected void searchFile() {
//		System.out.println("<searchFile>");

		list.removeAll();

		if (serverSocket == null || input == null || output == null) {
			System.out.println("searchFile - Error: " + "The connection is not established yet!");
			return;
		}

		try {
			String filename = text.getText();
			output.write(("SEARCH: " + filename + "\n").getBytes());
			BufferedReader br = new BufferedReader(new InputStreamReader(input));

			String s = null;

			while ((s = br.readLine()) != null) {
				if (s.isEmpty())
					break;

				System.out.println(s);
				
				if (s.equalsIgnoreCase("NOT FOUND"))
					break;

				if(s.startsWith("FOUND: "))
					s = s.substring(7);
				list.add(s);
				System.out.println(s);
			}
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}

//		System.out.println("</searchFile>");
	}

}
