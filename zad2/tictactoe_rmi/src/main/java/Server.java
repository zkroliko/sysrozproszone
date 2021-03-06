import java.rmi.Naming;
import java.rmi.server.UnicastRemoteObject;

public class Server {

    private static Playable game;

    public static void main(String[] args) {
        try {
            // to work in distributed system
            System.setProperty("java.rmi.server.hostname", "192.168.157.129");
            game = new Game();
            Playable playable = (Playable) UnicastRemoteObject.exportObject(game, 0);
            Naming.rebind( args[0], game );
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


}
