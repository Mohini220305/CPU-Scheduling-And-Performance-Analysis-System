package graph;
import java.util.*;

public class Graph {

    private Map<Node, List<Edge>> adjList;

    public Graph() {
        adjList = new HashMap<>();
    }

    public void addNode(Node node) {
        adjList.putIfAbsent(node, new ArrayList<>());
    }

    public void addEdge(Node source, Node dest, double distance) {
        adjList.putIfAbsent(source, new ArrayList<>());
        adjList.putIfAbsent(dest, new ArrayList<>());

        adjList.get(source).add(new Edge(dest, distance));
        adjList.get(dest).add(new Edge(source, distance)); // undirected
    }


    public List<Edge> getNeighbors(Node node) {
        return adjList.get(node);
    }

    public Set<Node> getAllNodes() {
        return adjList.keySet();
    }

    public void printGraph() {
        for (Node node : adjList.keySet()) {
            System.out.print(node + " -> ");
            for (Edge edge : adjList.get(node)) {
                System.out.print(edge.getDestination() +
                        "(" + edge.getWeight() + ") ");
            }
            System.out.println();
        }
    }
}
