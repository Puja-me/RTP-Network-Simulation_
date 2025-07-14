import matplotlib.pyplot as plt
import pandas as pd
import glob
import sys
import os

def plot_client_jitter(filename):
    """Plot jitter data from a client log file"""
    try:
        df = pd.read_csv(filename)

        plt.figure(figsize=(12, 8))

        # Plot buffer size over time
        plt.subplot(2, 1, 1)
        plt.plot(df['packet_id'], df['buffer_size'], 'b-', label='Buffer Size')
        plt.title(f'Jitter Buffer Size - {os.path.basename(filename)}')
        plt.xlabel('Packet ID')
        plt.ylabel('Buffer Size')
        plt.grid(True)
        plt.legend()

        # Plot processing time
        plt.subplot(2, 1, 2)
        plt.plot(df['packet_id'], df['processing_time_ms'], 'r-', label='Processing Time (ms)')
        plt.title(f'Packet Processing Time - {os.path.basename(filename)}')
        plt.xlabel('Packet ID')
        plt.ylabel('Processing Time (ms)')
        plt.grid(True)
        plt.legend()

        plt.tight_layout()

        # Save plot
        output_file = os.path.splitext(filename)[0] + '_plot.png'
        plt.savefig(output_file)
        print(f"Plot saved to {output_file}")

        plt.close()
    except Exception as e:
        print(f"Error processing {filename}: {e}")

def plot_server_jitter(filename):
    """Plot jitter data from the server log file"""
    try:
        df = pd.read_csv(filename)

        plt.figure(figsize=(12, 8))

        # Plot number of clients over time
        plt.subplot(3, 1, 1)
        plt.plot(range(len(df)), df['total_clients'], 'g-', label='Connected Clients')
        plt.title(f'Connected Clients - {os.path.basename(filename)}')
        plt.xlabel('Sample')
        plt.ylabel('Clients')
        plt.grid(True)
        plt.legend()

        # Plot average jitter
        plt.subplot(3, 1, 2)
        plt.plot(range(len(df)), df['avg_jitter_ms'], 'r-', label='Average Jitter (ms)')
        plt.title('Average Jitter')
        plt.xlabel('Sample')
        plt.ylabel('Jitter (ms)')
        plt.grid(True)
        plt.legend()

        # Plot total packets
        plt.subplot(3, 1, 3)
        plt.plot(range(len(df)), df['total_packets'], 'b-', label='Total Packets')
        plt.title('Total Packets Processed')
        plt.xlabel('Sample')
        plt.ylabel('Packets')
        plt.grid(True)
        plt.legend()

        plt.tight_layout()

        # Save plot
        output_file = os.path.splitext(filename)[0] + '_plot.png'
        plt.savefig(output_file)
        print(f"Plot saved to {output_file}")

        plt.close()
    except Exception as e:
        print(f"Error processing {filename}: {e}")

def plot_per_client_jitter(filename):
    """Plot per-client jitter data from server logs"""
    try:
        df = pd.read_csv(filename)

        plt.figure(figsize=(12, 8))

        # Plot jitter over time
        plt.subplot(2, 1, 1)
        plt.plot(df['packet_id'], df['jitter_ms'], 'b-', label='Jitter (ms)')
        plt.title(f'Network Jitter - {os.path.basename(filename)}')
        plt.xlabel('Packet ID')
        plt.ylabel('Jitter (ms)')
        plt.grid(True)
        plt.legend()

        # Plot delay over time
        plt.subplot(2, 1, 2)
        plt.plot(df['packet_id'], df['delay_ms'], 'r-', label='Total Delay (ms)')
        plt.title(f'Network Delay - {os.path.basename(filename)}')
        plt.xlabel('Packet ID')
        plt.ylabel('Delay (ms)')
        plt.grid(True)
        plt.legend()

        plt.tight_layout()

        # Save plot
        output_file = os.path.splitext(filename)[0] + '_plot.png'
        plt.savefig(output_file)
        print(f"Plot saved to {output_file}")

        plt.close()
    except Exception as e:
        print(f"Error processing {filename}: {e}")

def check_file_content(filename):
    """Check if file exists and has content"""
    try:
        if os.path.getsize(filename) > 0:
            return True
        else:
            print(f"File {filename} is empty.")
            return False
    except OSError:
        print(f"File {filename} doesn't exist or is inaccessible.")
        return False

def plot_all_files():
    """Find and plot all jitter log files"""
    # Process client jitter logs
    client_files = glob.glob("client_jitter_*.csv")
    for filename in client_files:
        if check_file_content(filename):
            print(f"Processing client file: {filename}")
            plot_client_jitter(filename)
    
    # Process server jitter logs
    server_files = glob.glob("server_stats.csv")
    for filename in server_files:
        if check_file_content(filename):
            print(f"Processing server file: {filename}")
            plot_server_jitter(filename)
    
    # Process per-client jitter logs from server
    jitter_files = glob.glob("jitter_*.csv")
    for filename in jitter_files:
        if check_file_content(filename):
            print(f"Processing jitter file: {filename}")
            plot_per_client_jitter(filename)  # Use the new function
    
    if not (client_files or server_files or jitter_files):
        print("No jitter log files found. Run the RTP server and clients first.")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        # Plot specific file
        filename = sys.argv[1]
        if not check_file_content(filename):
            sys.exit(1)

        if filename.startswith("client_jitter_"):
            plot_client_jitter(filename)
        elif filename == "server_stats.csv":
            plot_server_jitter(filename)
        elif filename.startswith("jitter_"):
            plot_per_client_jitter(filename)
        else:
            print(f"Unknown file format: {filename}")
    else:
        # Plot all files
        plot_all_files()
