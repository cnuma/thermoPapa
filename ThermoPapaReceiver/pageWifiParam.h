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
      Config Wifi
    </div>

    <div class="card-body">

      <FORM METHOD=GET ACTION=majWifi>
        <div class="row">
          <div class="col-5">
            Nom du réseau
          </div>
          <div class="col-7">
            <input name=ssid 	type=text size=20 maxlength=40>
          </div>
        </div>

        <div class="row">
            <div class="col-5">
              Mot de passe	
            </div>
            <div class="col-7">
              <input name=pwd 	type=text size=20 maxlength=40>
            </div>
        </div>

        <div class="row">
          <div class="col-12">
            <a href="" class="btn btn-primary">Sauvegarder la config.</a>
          </div>
        </div>

      </FORM>

      <div class="card-footer text-body-secondary">
        cNuma - 2023-11
      </div>

    </div>
  </div>

</body></html>)=====";9htFJ�