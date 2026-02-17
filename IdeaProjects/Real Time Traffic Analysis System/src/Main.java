import graph.*;
import algorithm.*;
import java.util.*;
import simulation.*;

public class Main {

    public static void main(String[] args) {

        Graph city = new Graph();

        Node A = new Node(1, "A");
        Node B = new Node(2, "B");
        Node C = new Node(3, "C");
        Node D = new Node(4, "D");
        Node E = new Node(5, "E");
        Node F = new Node(6, "F");

        city.addEdge(A, B, 5);
        city.addEdge(B, C, 3);
        city.addEdge(A, C, 1);
        city.addEdge(C, D, 6);
        city.addEdge(B, E, 4);
        city.addEdge(E, F, 2);

        Scanner sc = new Scanner(System.in);

        System.out.print("Enter source (A-F): ");
        String srcInput = sc.nextLine();

        System.out.print("Enter destination (A-F): ");
        String destInput = sc.nextLine();

        Node source = getNodeByName(city, srcInput);
        Node destination = getNodeByName(city, destInput);

        if (source == null || destination == null) {
            System.out.println("Invalid input!");
            return;
        }

// Update traffic
        TrafficSimulator simulator = new TrafficSimulator();
        simulator.updateTraffic(city);

// Get all paths
        AllPaths allPathsObj = new AllPaths();
        List<List<Node>> allPaths = allPathsObj.getAllPaths(city, source, destination);

        if (allPaths.isEmpty()) {
            System.out.println("No path found!");
            return;
        }

        System.out.println("\n===== AVAILABLE PATHS =====");

        int pathNumber = 1;

        for (List<Node> path : allPaths) {

            double totalCost = 0;

            System.out.print(pathNumber + ". ");

            for (int i = 0; i < path.size(); i++) {
                System.out.print(path.get(i) + " ");
            }

            // Calculate total cost
            for (int i = 0; i < path.size() - 1; i++) {
                Node from = path.get(i);
                Node to = path.get(i + 1);

                for (Edge edge : city.getNeighbors(from)) {
                    if (edge.getDestination().equals(to)) {
                        totalCost += edge.getWeight();
                    }
                }
            }

            System.out.println("| Total Cost: " + totalCost);
            pathNumber++;
        }

// Let user choose path
        System.out.print("\nSelect a path number: ");
        int choice = sc.nextInt();

        if (choice < 1 || choice > allPaths.size()) {
            System.out.println("Invalid choice!");
            return;
        }

        List<Node> selectedPath = allPaths.get(choice - 1);

        System.out.println("\nYou selected path: " + selectedPath);

// Move vehicle
        vehicle car1 = new vehicle("V1", source);
        car1.moveAlongPath(city, selectedPath);
    }

    public static Node getNodeByName(Graph graph, String name) {
        for (Node node : graph.getAllNodes()) {
            if (node.getName().equalsIgnoreCase(name)) {
                return node;
            }
        }
        return null;
    }
}
