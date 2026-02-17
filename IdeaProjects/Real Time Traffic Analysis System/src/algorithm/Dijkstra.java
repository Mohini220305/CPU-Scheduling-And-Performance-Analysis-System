package algorithm;

import graph.*;
import java.util.*;

public class Dijkstra {

    public void findShortestPath(Graph graph, Node source, Node destination) {

        Map<Node, Double> distances = new HashMap<>();
        Map<Node, Node> previous = new HashMap<>();

        PriorityQueue<Node> pq = new PriorityQueue<>(
                Comparator.comparingDouble(distances::get)
        );

        // Initialize
        for (Node node : graph.getAllNodes()) {
            distances.put(node, Double.MAX_VALUE);
        }

        distances.put(source, 0.0);
        pq.add(source);

        while (!pq.isEmpty()) {

            Node current = pq.poll();

            for (Edge edge : graph.getNeighbors(current)) {

                Node neighbor = edge.getDestination();
                double newDist = distances.get(current) + edge.getWeight();

                if (newDist < distances.get(neighbor)) {
                    distances.put(neighbor, newDist);
                    previous.put(neighbor, current);
                    pq.remove(neighbor);
                    pq.add(neighbor);
                }
            }
        }

        // If destination unreachable
        if (!distances.containsKey(destination) ||
                distances.get(destination) == Double.MAX_VALUE) {
            System.out.println("No path found!");
            return;
        }

        // Reconstruct Path
        List<Node> path = new ArrayList<>();
        Node step = destination;

        while (step != null) {
            path.add(step);
            step = previous.get(step);
        }

        Collections.reverse(path);

        // Print Result
        System.out.println("\n===== ROUTE DETAILS =====");
        System.out.println("Source: " + source);
        System.out.println("Destination: " + destination);
        System.out.println("Total Distance/Time: " + distances.get(destination));

        System.out.print("Path: ");
        for (Node node : path) {
            System.out.print(node + " ");
        }
        System.out.println();

        // Print traffic info for each segment
        System.out.println("\nTraffic Details:");
        for (int i = 0; i < path.size() - 1; i++) {
            Node current = path.get(i);
            Node next = path.get(i + 1);

            for (Edge edge : graph.getNeighbors(current)) {
                if (edge.getDestination().equals(next)) {
                    System.out.println(current + " → " + next +
                            " | Traffic: " + edge.getTrafficLevel() +
                            " | Weight: " + edge.getWeight());
                }
            }
        }
    }
    public List<Node> getShortestPath(Graph graph, Node source, Node destination) {

        Map<Node, Double> distances = new HashMap<>();
        Map<Node, Node> previous = new HashMap<>();

        PriorityQueue<Node> pq = new PriorityQueue<>(
                Comparator.comparingDouble(distances::get)
        );

        for (Node node : graph.getAllNodes()) {
            distances.put(node, Double.MAX_VALUE);
        }

        distances.put(source, 0.0);
        pq.add(source);

        while (!pq.isEmpty()) {

            Node current = pq.poll();

            for (Edge edge : graph.getNeighbors(current)) {

                Node neighbor = edge.getDestination();
                double newDist = distances.get(current) + edge.getWeight();

                if (newDist < distances.get(neighbor)) {
                    distances.put(neighbor, newDist);
                    previous.put(neighbor, current);
                    pq.remove(neighbor);
                    pq.add(neighbor);
                }
            }
        }

        List<Node> path = new ArrayList<>();
        Node step = destination;

        while (step != null) {
            path.add(step);
            step = previous.get(step);
        }

        Collections.reverse(path);
        return path;
    }

}
