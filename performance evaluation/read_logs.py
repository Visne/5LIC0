## imports
import os
import sys

import pyshark
import numpy as np
import matplotlib.pyplot as plt
from copy import deepcopy
import argparse


### hall of data structure horros
#buffer the packets from file
pkt_list = []
COAP_list = []
WPAN_list = []
ICMPV6_list = []

class paired_packets:
    def __init__(self):
        self.message_id = 'mid' #ID of message (should be common between request and response
        self.message_uri = 0;  #can be blank or contain data
        self.message_code = 0
        #data transmitted and recieved
        self.message_request_payload_raw = 0
        self.message_respobse_payload_raw = 0
        #raw packet objects
        self.request = 'req_obj'  #initial request sent out by the client node (packet object)
        self.response = 'res_obj'  #final acknoledged response by the server (packet object)
        #signal characteristics
        #timestamp calculations
        self.init_ts = 0 #unix timesamp of initial transmission from server
        self.final_ts = 0 #unix timestap of final response back to client
        self.delay_ts = 0 #difference between above 22 unix timestamps, absolute
        self.client_transmits = 0 #how many times did the client node need to send the packet before good?
        self.server_transmits = 0 #how many times did the server node need to send the packet before good?
        #used to store the packet delivery ratio for this transaction
        self_txPDR = 1 




###loader and processing functions

#helper functions
def check_if_ordered(packets):
    ts_o = 0
    for t in range(len(packets)):
        ts_n = float(packets[t].sniff_timestamp)
        if ts_n >= ts_o: #if packet is newer
            #keep going
            #print(ts_n)
            ts_o = ts_n
            continue
        else:
            return False
    return True
            
def cull_aberrations(transactions): #note: some paired up transactions dont get properly paired up if final response isnt included in the file. When this happens, the recorded latency is equal to the initial timestamp, and im making the assumption that the delay isnt equal to several decades :p. We need to remove these from consideration.
    i = 0 #counter to iterate thru file, keeping in mind we will be removing data
    nelem = len(transactions)
    while (i < nelem):
        if (transactions[i].delay_ts > 500): #(seems arbitrarily large enough)
            transactions.remove(transactions[i])
            nelem -= 1
        else:
            i += 1 
    return transactions


def pairup(packets): #takes a list of COAP packets and pairs up initial client requests with (final/acknowledged) server responses
    
    pairs = list()
    message_ids = []
    for i in range(len(packets)): #iterate thru all packets in log
        if((i%1000) == 0):
            print(f"Processing element {i} out of {len(packets)}")
        
        mID = int(packets[i].coap.mid) #message id
        msg_ts = float(packets[i].sniff_timestamp)  #unix TS of packet, used later
        msg_code = int(packets[i].coap.code)
        #print((mID in message_ids))
        if (not(mID in message_ids)): #if transaction hasn't yet been recorded --> new REQ/RES pair
            #record new ID
            pairs.append(deepcopy(paired_packets())) #add new element to message id database
            path = packets[i].coap.opt_uri_path_recon
            #print(path)
            pairs[-1].message_id = mID
            message_ids.append(mID)
            #lets try to extract info about this message
            #code = int(packets[i].coap.code) #message is either of type 1 (GET), 2 (POST) or 69 (CONtent)
            
            pairs[-1].request = packets[i].coap
            pairs[-1].message_uri = path
            pairs[-1].message_code = msg_code
            pairs[-1].client_transmits += 1
            pairs[-1].init_ts = msg_ts

            #pairs.append(p) #add new element
            
        else: #message ID has already been encountered, there is an object p in pair array with the' same message ID
        #slow and dumb approach
            #find location in storage structure
            
            #STEPS: we now need to find the last packet of same message id for comparison and we need to find the lcoation at which the packet pair is being stored in pairs[]. 
            #location found by iterating through pairs[] until we find the first element with same message id
            #for last message, we iterate backwards thru packet list until we find newest element of same msg ID. We then compare codes to decide if this is a retransmission or response. Once found we rbeak and move on. 
            #ASSUMES packets are temporaly ordered

            
            p_loc = 0
            for v in range(i): #find first instance of packet with same Id
                msg_id_first = int(message_ids[v])
                if (msg_id_first == mID): #if match,record position
                    p_loc = v
                    break
            #print(p_loc)

            for k in reversed(range(i)): 
            #find placement of youngest past packet with same message ID for comparison
            #if a request with matching id and code, increment client_transmits or server_transmits respectively depending on code (packet has been resent)
            #if this is a message of same ID but different code, it's likely a server response
                msg_id_old = int(packets[k].coap.mid)
                msg_code_old = int(packets[k].coap.code)
                #print(f"({i}:{k} and {msg_code}:{msg_code_old} and {mID}:{msg_id_old}")
                
                #different cases are handled here
                #CASE A: packet is simply a retransmission after initial failed transmission --> GET or POST
                if ((msg_id_old == mID) and (msg_code_old == msg_code) and (msg_code == 1 or msg_code == 2)): #NB : location of struct containing matching data is at pairs[p_loc]  
                    pairs[p_loc].client_transmits += 1 
                    #print('case A')
                    break
                #CASE B: packet is simply a retransmission after initial failed transmission --> RESPONSE
                elif ((msg_id_old == mID) and (msg_code_old == msg_code) and (msg_code == 69)): #NB : location of struct containing matching data is at pairs[p_loc]
                    pairs[p_loc].server_transmits += 1 
                    pairs[p_loc].final_ts = msg_ts #packet is a response --> write down highest (yet) timestamp
                    #print('case B')
                    break
                #CASE C: packet is a response to a previous requestn eg server response
                elif ((msg_id_old == mID) and (msg_code_old != msg_code) and (msg_code == 69 )): #same message ID but NOT same type: --> response
                    pairs[p_loc].server_transmits += 1 
                    pairs[p_loc].response = packets[i].coap
                    pairs[p_loc].final_ts = msg_ts #packet is a response --> write down highest (yet) timestamp
                    #print('case C')
                    break
                #CASE D: no match
                elif ((msg_id_old != mID)): 
                    continue
                else: #case E, other (possibly server went first??
                    pairs[p_loc].server_transmits += 1 
                    pairs[p_loc].final_ts = msg_ts
                    break
                    
                    
        for i in range(len(pairs)): #compute delay
            pairs[i].delay_ts = abs(pairs[i].final_ts - pairs[i].init_ts) 
            #sanity  check: should never exceed 100%
            if (pairs[i].server_transmits != 0) and (pairs[i].client_transmits != 0 ):
                pairs[i].txPDR = 2/(pairs[i].server_transmits + pairs[i].client_transmits)
            else: 
                pairs[i].txPDR = 'invalid'
            
            

    return pairs

### performance eval functions

def extract_latency(transactions):
    latency_measurements_all = []
    latency_measurements_query = []
    latency_measurements_scan = []
    latency_measurements_update = []
    latency_warn_list = []
    #contains the percentage of transactions with e2e latency measured under 2s
    frac_all_2s = 0
    frac_scan_2s = 0
    frac_query_2s = 0
    frac_update_2s = 0
    
    for tx in transactions: #iterate thru transaction pair log
        tx_latency = tx.delay_ts
        if tx_latency > 5: #s
            latency_warn_list.append(transactions) #mostly used for debugging
        latency_measurements_all.append(tx_latency)
        if tx_latency < 2: 
           frac_all_2s += 1
           
        if tx.message_code == 1:
            latency_measurements_query.append(tx_latency)
            if tx_latency < 2: 
                frac_query_2s += 1
        elif tx.message_code == 2:
            latency_measurements_scan.append(tx_latency)
            if tx_latency < 2: 
                frac_scan_2s += 1
        elif tx.message_code == 69:
            latency_measurements_update.append(tx_latency)
            if tx_latency < 2: 
                frac_update_2s += 1
    try:        
        frac_all_2s = frac_all_2s/len(latency_measurements_all)
    except:
        frac_all_2s = 1
    try:
        frac_query_2s = frac_query_2s/len(latency_measurements_query)
    except:
        frac_query_2s = 1
    try:
        frac_scan_2s = frac_scan_2s/len(latency_measurements_scan)
    except:
        frac_scan_2s = 1
    try:
        frac_update_2s = frac_update_2s/len(latency_measurements_update)
    except:
        frac_update_2s = 1
    #some statistics
    #np.mean(latency_measurements)
    return latency_measurements_all, latency_measurements_query, latency_measurements_scan, latency_measurements_update, latency_warn_list, frac_all_2s, frac_query_2s,frac_scan_2s,frac_update_2s
    
def latency_statistics(latency_measurements_all, latency_measurements_query, latency_measurements_scan, latency_measurements_update):
    scan_latency = {'average':0 , 'median':0, 'std_dev':0}
    query_latency = {'average':0 , 'median':0, 'std_dev':0}
    update_latency = {'average':0 , 'median':0, 'std_dev':0}
    all_latency = {'average':0 , 'median':0, 'std_dev':0}
    #compute stats for all data
    all_latency['average'] = np.average(latency_measurements_all)
    all_latency['median'] = np.median(latency_measurements_all)
    all_latency['std_dev'] = np.std(latency_measurements_all)
    #compute stats for queries
    query_latency['average'] = np.average(latency_measurements_query)
    query_latency['median'] = np.median(latency_measurements_query)
    query_latency['std_dev'] = np.std(latency_measurements_query)
    #compute stats for scans
    scan_latency['average'] = np.average(latency_measurements_scan)
    scan_latency['median'] = np.median(latency_measurements_scan)
    scan_latency['std_dev'] = np.std(latency_measurements_scan)
    #compute stats for all updates
    update_latency['average'] = np.average(latency_measurements_update)
    update_latency['median'] = np.median(latency_measurements_update)
    update_latency['std_dev'] = np.std(latency_measurements_update)
    
    
    return all_latency, query_latency, scan_latency, update_latency
            

def compute_throughput(transactions, N_tx, N_query, N_scan, N_update): #computes throughput figures for all transaction types as well as number of achieved packets for each service. Takes a culled list of paired packets as input
    init_timesamps = []
    final_timestamps = []
    for i in range(len(transactions)):
        ts1 = transactions[i].init_ts
        ts2 = transactions[i].final_ts
        #sanity check: final ts should be greater than start ts and both fields should be populated, but in case it isnt reorder (errors here would not affect latency figure estimations)
        if (ts2 > ts1) and (ts1 != 0) and (ts2 != 0) :
            init_timesamps.append(ts1)
            final_timestamps.append(ts2)
        elif (ts2 < ts1) and (ts1 != 0) and (ts2 != 0):
            init_timesamps.append(ts2)
            final_timestamps.append(ts1)
        else: 
            continue
    start_of_file_ts = min(init_timesamps)
    end_of_file_ts = max(final_timestamps)
    file_duration = end_of_file_ts - start_of_file_ts
    
    scan_throughput = N_scan/file_duration
    query_throughput = N_query/file_duration
    update_throughput = N_update/file_duration
    total_thrughput = N_tx/file_duration  
    
    return total_thrughput, query_throughput, scan_throughput, update_throughput, file_duration    
    
def compute_pdr(transactions): #computes the PDR over the entirety of the recording by loooking of number of transmissions:
    client_transmits_sum = 0
    server_transmits_sum = 0
    for i in range(len(transactions)):
        #how many transactions were sent out?
        client_transmits_sum += transactions[i].client_transmits
        server_transmits_sum += transactions[i].server_transmits
    total_transactions_recorded = client_transmits_sum + server_transmits_sum #should be roughly equal to packet list size
    #PDR = N_delivered / N_transmitted. Assume that each acknoledged packet is not retransmitted (PDR = 100% -> 2 transmissions per transaction)
    successful_transactions = len(transactions) # only paired-up/successful packets in transaction list (cull applied)
    PDR = (2*successful_transactions) / total_transactions_recorded
    return PDR, client_transmits_sum, server_transmits_sum, successful_transactions
        
    

### main loop

def mainloop(filename, node_count, bincount):
    
    sys.argv
    #os.chdir(str(sys.path[0]))
    print("Set directory to: {}\n".format(os.getcwd()))
    #filedir = 'radiolog-perf50node-2hr.pcap'

    # Load the pcap file
    cap = pyshark.FileCapture(filename)


    # Iterate over packets to extract CoAP information
    print("Loading data...")

    load_idx = 0 #used to indicate progress (loads can be long on big radio logs)
    for packet in cap:
        if 'COAP' in packet:
            load_idx += 1
            COAP_list.append(packet)
            if (load_idx % 1000) == 0:
                print(f"Loaded {load_idx} packets")
    print(f"Loaded {load_idx} COAP packets. Done.")
    if not(check_if_ordered(COAP_list)):
        print("ERROR: packets in database not ordered, something went wrong!")
        quit()
    list_of_transactions = pairup(COAP_list)
    #remove junk data at file edges
    list_of_transactions = cull_aberrations(list_of_transactions)

    #compute latency
    latency_measurements_all, latency_measurements_query, latency_measurements_scan, latency_measurements_update, latency_warn_list, frac_all_2s,       frac_query_2s,frac_scan_2s,frac_update_2s = extract_latency(list_of_transactions)
    
    #stats for latency
    all_latency, query_latency, scan_latency, update_latency = latency_statistics(latency_measurements_all, latency_measurements_query, latency_measurements_scan, latency_measurements_update)
    
    #compute throughput
    total_throughput, query_throughput, scan_throughput, update_throughput, file_duration  = compute_throughput(list_of_transactions, len(latency_measurements_all), len(latency_measurements_query), len(latency_measurements_scan), len(latency_measurements_update))
    
    print("\n### THROUGHPUT ###\n")
    print(f"Total throughput: {np.around(total_throughput, 4)} TX/sec")
    print(f"Scan throughput: {np.around(scan_throughput, 4)} TX/sec")
    print(f"Query throughput: {np.around(query_throughput, 4)} TX/sec")
    print(f"Update throughput: {np.around(update_throughput, 4)} TX/sec")
    
    print("\n### PDR ###")
    #compute PDR
    (pdr, Nc, Ns, Nsuc) = compute_pdr(list_of_transactions)
    print(f"Achieved PDR over recording: {np.around(pdr*100,2)}% [{2*Nsuc}/({Nc}+{Ns})]")
    
    print("\n### LATENCY ###")
    
    print(f"All services: \n- average: {np.around(all_latency['average'], 3)}s \n- median: {np.around(all_latency['median'], 4)}s \n- std deviation: {np.around(all_latency['std_dev'], 3)}s \n- share of packets with latency <2s: {np.around(frac_all_2s*100, 2)}%")
    
    print(f"Query: \n- average: {np.around(query_latency['average'], 3)}s \n- median: {np.around(query_latency['median'], 4)}s \n- std deviation: {np.around(query_latency['std_dev'], 3)}s \n- share of packets with latency <2s: {np.around(frac_query_2s*100, 2)}%")
    print(f"Scan: \n- average: {np.around(scan_latency['average'], 3)}s \n- median: {np.around(scan_latency['median'], 4)}s \n- std deviation: {np.around(scan_latency['std_dev'], 3)}s \n- share of packets with latency <2s: {np.around(frac_scan_2s*100, 2)}%")
    print(f"Update services: \n- average: {np.around(update_latency['average'], 3)}s \n- median: {np.around(update_latency['median'], 4)}s \n- std deviation: {np.around(update_latency['std_dev'], 3)}s \n- share of packets with latency <2s: {np.around(frac_update_2s*100, 2)}%")
    
    #display latency graphs
    fig, axs = plt.subplots(2, 2)

    nodecount = node_count
    
    #adjust bin count as needed for visualization, weights normalize data so we get percentages

    axs[0, 0].hist(latency_measurements_all, bins = bincount,  weights= np.ones(len(latency_measurements_all))*100 / len(latency_measurements_all))
    axs[0, 0].set_title(f"Latency of all transactions (median: {np.around(all_latency['median'], 3)}s, <2s: {np.around(frac_all_2s*100,1)}% )")
    axs[0, 0].set_xlabel("e2e latency (s)")
    axs[0, 0].set_ylabel("% of packets ")
    axs[0, 0].set_xlim([0, 6])

    axs[0, 1].hist(latency_measurements_scan, bins = int(bincount/2), color= 'green',weights= np.ones(len(latency_measurements_scan))*100 / len(latency_measurements_scan))
    axs[0, 1].set_title(f"Latency of scan transactions (median: {np.around(scan_latency['median'], 3)}s, <2s: {np.around(frac_scan_2s*100,1)}%)")
    axs[0, 1].set_xlim([0, 6])
    axs[0, 1].set_xlabel("e2e latency (s)")
    axs[0, 1].set_ylabel("% of packets ")

    axs[1, 0].hist(latency_measurements_query, bins = int(bincount/2),  color='orange', weights= np.ones(len(latency_measurements_query))*100 / len(latency_measurements_query))
    axs[1, 0].set_title(f"Latency of query transactions (median: {np.around(query_latency['median'], 3)}s , <2s: {np.around(frac_query_2s*100,1)}%)")
    axs[1, 0].set_xlim([0, 6])
    axs[1, 0].set_xlabel("e2e latency (s)")
    axs[1, 0].set_ylabel("% of packets ")

    axs[1, 1].hist(latency_measurements_update, bins= int(bincount/20), color='red', weights= np.ones(len(latency_measurements_update))*100 / len(latency_measurements_update))
    axs[1, 1].set_title(f"Latency of update transactions (median: {np.around(update_latency['median'], 3)}s, <2s: {np.around(frac_update_2s*100,1)}%)")
    axs[1, 1].set_xlim([0, 6])
    axs[1, 1].set_xlabel("e2e latency (s)")
    axs[1, 1].set_ylabel("% of packets ")

    fig.suptitle(f"End-to-end latency of network with {nodecount} nodes for different transaction types")
    
    print("\n### MISC ###" )
    try:
        print(f"File length {file_duration}s")
    except:
        print("Metadata unavailable")
    cap.close() #close radio log
    plt.show()

if __name__ == "__main__":
    commands = [arg for arg in sys.argv]
    if len(commands) != 4: 
        print("CLI Argument error -> args should be <pcap_file> <node_count> <bin_count_histogram>")
        #print(commands)
        quit()
    else:
        commands[1] = str(commands[1])
        commands[2] = int(commands[2])
        commands[3] = int(commands[3])
        mainloop(commands[1],commands[2],commands[3])


