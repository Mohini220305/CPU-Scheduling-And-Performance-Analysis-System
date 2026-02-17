package algorithm;

import graph.*;
import java.util.*;

public class bfs {

    public void traverse(Graph graph, Node start) {

        Set<Node> visited = new HashSet<>();
        Queue<Node> queue = new LinkedList<>();

        visited.add(start);
        queue.add(start);

        System.out.println("BFS Traversal:");

        while (!queue.isEmpty()) {

            Node current = queue.poll();
            System.out.print(current + " ");

            for (Edge edge : graph.getNeighbors(current)) {
                Node neighbor = edge.getDestination();

                if (!visited.contains(neighbor)) {
                    visited.add(neighbor);
                    queue.add(neighbor);
                }
            }
        }
        System.out.println();
    }
}
