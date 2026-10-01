import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as MatplotPolygon
import os
import sys
import numpy as np

def plot_fig_solution(filename,filenameOut):
    fig, ax = plt.subplots(figsize=(12, 6))
    
    with open(filename, 'r') as f:
        # 1. Read the Board Dimensions (First Line: L H)
        header = f.readline().split()
        if not header: return
        L, H = float(header[0]), float(header[1])
        
        # Draw the board (the boundary)
        board_rect = plt.Rectangle((0, 0), L, H, linewidth=2, 
                                   edgecolor='black', facecolor='none', ls='--')
        ax.add_patch(board_rect)

	# Vertical lines
        for x in np.arange(0, L, 1):
            ax.axvline(
                x=x, color="red", linestyle="--", linewidth=1.2, alpha=0.8
            )

        # Horizontal lines
        for y in np.arange(0, H, 1):
            ax.axhline(
                y=y, color="red", linestyle="--", linewidth=1.2, alpha=0.8
            )

        # 2. Process each polygon line
        for line in f:
            parts = line.split()
            if len(parts) < 3: continue
            
            poly_id = int(parts[0])
            coords = [float(x) for x in parts[1:]]
            
            # Zip x and y into (x, y) tuples
            pts = list(zip(coords[0::2], coords[1::2]))
            
            if pts:
                # 3. Create and add the polygon patch
                # Using a color map to differentiate polygons by ID
                color = plt.cm.tab10(poly_id % 10)
                
                poly_patch = MatplotPolygon(pts, closed=True, 
                                            edgecolor='black', 
                                            facecolor=color, 
                                            alpha=1, 
                                            linewidth=0.5)
                ax.add_patch(poly_patch)
                
    # Formatting the plot
    ax.set_xlim(0, L)
    ax.set_ylim(0, H)
    ax.set_aspect('equal')
    plt.grid(True, linestyle=':', alpha=0.5)
    
    filenameOut += "_L" + str(int(L)) + ".png"
    # 5. Save the file as PNG
    plt.savefig(filenameOut, dpi=300, bbox_inches='tight')
    plt.show()

# Run it
plot_fig_solution(sys.argv[1],sys.argv[2])
