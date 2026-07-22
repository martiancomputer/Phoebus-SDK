


/*
  init:rtk_modeltest_init()
  
	rtk_test_user_pipe_cmd("echo 1 > proc/fc/ctrl/flow_skipAllPsTracking");
	rtk_test_user_pipe_cmd("echo 1 > proc/fc/ctrl/flow_l2_skipPsTracking");

	DataPath:
	Test case gen  ingress/egress packet and fill (rtk_fcTesting_case_t *info)
	-> DirecTx(using headera [lspid/streamid ...] )  : prepare ingress input packet
	-> Nic Rx 
	-> FC ingress (rtk_fc_ingress_flowLearning)
	-> FC ingress hook netif_rx(rtk_fc_skb_fctestHook_rxhook_register(rtk_fcTesting_netif_rx);) : prepare egress packet
	-> FC egress (rtk_fc_egress_flowLearning) : Create hardware
	
*/


