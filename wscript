def build(bld):
    # Define the hello-ns3 program
    bld.program(source='hello-ns3.cc', target='hello-ns3')

    # Define the RTP server program
    bld.program(
        source=['rtp-server-main1.cc', 'rtp-server.cc'],
        target='rtp-server-main1',
        use=['core', 'network', 'internet', 'point-to-point', 'applications']
    )

    # Define the RTP client program
    bld.program(
        source=['rtp-client-main.cc', 'rtp-client.cc'],
        target='rtp-client-main',
        use=['core', 'network', 'internet', 'point-to-point', 'applications']
    )

