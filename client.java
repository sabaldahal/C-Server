import java.io.* ;
import java.net.* ;
import java.util.* ;
import java.util.Scanner;

public class client {
    public static void main(String argv[]) throws Exception{
        String sendtoserver = "";
        boolean connectedToServer = false;
        BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
        Scanner myObj = new Scanner(System.in);
        Socket clientSocket = null;
        DataOutputStream outToServer = null ;
        BufferedReader inFromServer = null;
        
        

        while(!sendtoserver.equalsIgnoreCase("exit")){
            sendtoserver = myObj.nextLine();
            StringTokenizer tokens = new StringTokenizer(sendtoserver);
            String command = tokens.nextToken();
            if(command.equalsIgnoreCase("Connect") && (connectedToServer == false)){
                try{
                    String addr = tokens.nextToken();
                    String port = tokens.nextToken();
                    int portnum = Integer.parseInt(port);
                    clientSocket = new Socket(addr, portnum);
                    outToServer = new DataOutputStream(clientSocket.getOutputStream());
                    inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                    ListenFromServer clientlisten = new ListenFromServer(inFromServer);
                    connectedToServer = true;
                    Thread thread = new Thread(clientlisten);
                    thread.start();
                    sendtoserver = "PING " + sendtoserver + "\r\n";
                    outToServer.write(sendtoserver.getBytes());
                    
                }catch(Exception e){
                    System.out.println("Self: Invalid Arguments");
                    continue;
                }

            }else if(command.equalsIgnoreCase("exit") && (connectedToServer == true)){
                sendtoserver = "UCMD " + sendtoserver + "\r\n";
                outToServer.write(sendtoserver.getBytes());
                sendtoserver = "exit";

            }else if(connectedToServer == true){
                sendtoserver = "UCMD " + sendtoserver + "\r\n";
                outToServer.write(sendtoserver.getBytes());
            }            
        }
        if(connectedToServer){
            clientSocket.close();
        }      
    }
}

final class ListenFromServer implements Runnable{
    BufferedReader reader;

    public ListenFromServer(BufferedReader reader) throws Exception{
        this.reader = reader;
    }

    
    @Override
    public void run(){
        try{
            processRequest();
        } catch (Exception e){
            System.out.println(e);
        }
    }

    void processRequest() throws Exception{
        String infromserver;
        while((reader != null) && (infromserver = reader.readLine()) != null){
            System.out.println(infromserver);
        }
    }
}

