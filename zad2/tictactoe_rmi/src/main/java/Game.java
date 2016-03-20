import java.rmi.RemoteException;
import java.util.*;

public class Game implements Playable {

    public static final int MAX_PLAYERS = 2;

    private Board board;

    private List<Player> players;

    private Map<Player, Bot> bots;

    private List<IGameListener> listeners;

    private Player currentPlayer;

    private Player winner;

    private boolean gameRunning = false;

    public Game() {
        board = new Board();
        players = new ArrayList<Player>();
        listeners = new ArrayList<IGameListener>();
        bots = new HashMap<Player, Bot>();
    }

    public synchronized void register(Player player, IGameListener listener) throws RemoteException {
        if (players.size() < MAX_PLAYERS) {
            players.add(player);
            listeners.add(listener);
            if (players.size() == 1) {
                currentPlayer = player;
                System.out.println("Current player: " + currentPlayer.getNick());

                // Making it easy on human player
                checkBots();
            }
        } else {
            System.err.println("Not enough room for: " + currentPlayer.getNick());
        }
        if (players.size() == MAX_PLAYERS) {
            gameRunning = true;
            System.err.println("Running game with: " + players.size() + " players");

        }
    }

    public synchronized void makeMove(Player player, Coordinates coordinates) throws RemoteException {
        if (gameRunning) {
            System.out.println("Moving and current player is: " + currentPlayer.getNick());
            if (gameRunning && player.getNick().equals(currentPlayer.getNick())) {
                if (board.getField(coordinates) == Board.EMPTY) {
                    board.setField(coordinates, player.getSymbol());

                    try {
                        // Checking for victory
                        if (VictorySolver.victoryExists(board, players)) {
                            winner = VictorySolver.findWinner(board, players);
                            gameRunning = false;
                            System.out.println("Winner is : " + winner.getNick());
                        } else {
                            // No victory
                            chooseNextPlayer();
                            notifyPlayers();
                        }
                    } catch (PlayerNotFoundException e) {
                        e.printStackTrace();
                    }
                }
            } else {
                System.out.println("Bad move by: " + currentPlayer.getNick());
            }

        } else {
            System.out.println(String.format("Player %s tried to move but game is not running", player.getNick()));
            }
        }

    private void notifyPlayers() throws RemoteException {
        for (IGameListener l: listeners) {
            l.onMove();
        }
    }

    private void chooseNextPlayer() {
        int currentIndex = players.indexOf(currentPlayer);
        currentPlayer = players.get((currentIndex+1) % players.size());
        System.out.println("Current player: " + currentPlayer.getNick());
        checkBots();
    }

    private void checkBots() {
        if (bots.containsKey(currentPlayer)) {
            System.out.println("Bot moving: " + currentPlayer.getNick());
            moveBot(bots.get(currentPlayer));
        }
    }

    private void moveBot(Bot bot) {
        try {
            bot.makeMove();
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NegativeArgumentException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void fillWithBots() throws RemoteException {
        while(players.size() < MAX_PLAYERS) {
            Player botPlayer = Bot.makePlayer('b');
            register(botPlayer, new GameListener());
            bots.put(botPlayer,new Bot(this,botPlayer));
        }
    }

    public Board getBoard() throws RemoteException{
        return board;
    }

    public Player getCurrentPlayer() throws RemoteException {
        return currentPlayer;
    }

    @Override
    public boolean isRunning() {
        return gameRunning;
    }

}
