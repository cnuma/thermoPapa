const char pageWifiParam[] PROGMEM = R"=====(
<html>
<head>
  <meta http-equiv="content-type" content="text/html;charset=UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous">
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js" integrity="sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL" crossorigin="anonymous"></script>
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
	<title>#cNuma Prod - Home page</title>
</head>
<body>

  <div class="card text-center">
    <div class="card-header">
      <H1>Thermo Papa</H1><P><H2>Config Wifi</H2>
    </div>

    <div class="card-body">

      <form method="POST" action="/WifiSave">

        <div class="form-group">
          <label for="SSID">SSID</label>
          <input type="text" class="form-control" id="ssid" name="ssid" aria-describedby="ssidHelp" placeholder="SSID">
          <small id="ssidHelp" class="form-text text-muted">Entrer le SSID du réseau local (max 40 caractères)</small>
        </div>

        <div class="form-group">
          <label for="pwd">Password</label>
          <input type="text" class="form-control" id="pwd" name="pwd" aria-describedby="pwdHelp" placeholder="Password">
          <small id="pwdHelp" class="form-text text-muted">Entrer le mot de passe associé au SSID du réseau local (max 40 caractères)</small>
        </div>

        <button type="submit" class="btn btn-primary" id="submitBtn">Submit</button>
      </form>



      <div class="card-footer text-body-secondary">
        <A href="/contact">cNuma - 2023-11</A>
      </div>

    </div>
  </div>

</body>
</HTML>

)=====";