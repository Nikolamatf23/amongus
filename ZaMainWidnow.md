- [ ] PLS UBACITE
- [ ]connect(
  m_client,
  &Client::messageReceived,
  m_server,
  [server = m_server, client = m_client](const Message& msg)
  {
  if (msg.type() == MessageType::JoinAccepted) {
  server->setHostId(client->myId());
  }
  }
  );void MainWindow::onHostJoinAccepted(const Message& msg)
  {
  if (msg.type() != MessageType::JoinAccepted ||
  m_server == nullptr) {
  return;
  }

  m_server->setHostId(m_client->myId());
  }
- [ ] ovo je da bi server imao setovano polje ko je host odnosno koji je hostId 
- [ ] to ce biti prvi igrac koji se poveze sa serverom