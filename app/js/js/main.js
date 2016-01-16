/* global $ */

$(function () {
  // Hide alert and disable button

  $('#submit').attr('disabled', '');
  $('.alert').hide();

  var hostname = window.location.hostname;
  if (hostname !== '127.0.0.1' && hostname !== 'localhost') {
    $('#homie_host').val(hostname);
  }

  // Variable declaration

  var url = 'http://homie.config';

  var hiddenNetwork = false;
  var openNetwork = false;

  var availableNetworksWorker;

  var stopNetworksWorker = function () {
    clearTimeout(availableNetworksWorker);
  };

  var getAvailableNetworks = function () {
    $.ajax({
      url: url + '/networks',
      method: 'GET',
      dataType: 'json'
    }).done(function (data) {
      $('.alert').hide();
      $('#networks_loading').hide();
      stopNetworksWorker();
      $('#submit').removeAttr('disabled');

      var networks = data.networks;

      networks.sort(function (networkA, networkB) {
        if (networkA.rssi > networkB.rssi) {
          return -1;
        } else if (networkA.rssi < networkB.rssi) {
          return 1;
        } else {
          return 0;
        }
      });

      networks.forEach(function (network) {
        var signalQuality;
        if (network.rssi <= -100) {
          signalQuality = 0;
        } else if (network.rssi >= -50) {
          signalQuality = 100;
        } else {
          signalQuality = 2 * (network.rssi + 100);
        }

        var signalQualityColor;
        if (signalQuality >= 66) {
          signalQualityColor = 'success';
        } else if (signalQuality >= 33) {
          signalQualityColor = 'warning';
        } else {
          signalQualityColor = 'danger';
        }

        var encryption;
        switch (network.encryption) {
          case 'wep':
            encryption = 'WEP';
            break;
          case 'wpa':
            encryption = 'WPA';
            break;
          case 'wpa2':
            encryption = 'WPA2';
            break;
          case 'none':
            encryption = 'Open';
            break;
          case 'auto':
            encryption = 'Automatic';
            break;
        }

        var builder = [];
        builder.push('<a href="#" class="list-group-item">');
        builder.push('  <h4 class="list-group-item-heading">' + network.ssid + '</h4>');
        builder.push('  <p class="list-group-item-text">');
        builder.push('    <div class="progress">');
        builder.push('      <div class="progress-bar progress-bar-' + signalQualityColor + '" role="progressbar" aria-valuenow="' + signalQuality + '" aria-valuemin="0" aria-valuemax="100" style="min-width: 6em; width: ' + signalQuality + '%;">');
        builder.push('        Signal: ' + signalQuality + '%');
        builder.push('      </div>');
        builder.push('    </div>');
        builder.push('    <span class="glyphicon glyphicon-lock"> ' + encryption + '</span>');
        builder.push('  </p>');
        builder.push('</a>');

        $('#networks').append(builder.join('\n'));
      });

      var builder = [];
      builder.push('<a href="#" class="list-group-item" id="hidden_network">');
      builder.push('  <h4 class="list-group-item-heading">Hidden network</h4>');
      builder.push('  <p class="list-group-item-text">');
      builder.push('    <p>Your network might be configured to not advertise itself. Select this option if this is the case.</p>');
      builder.push('  </p>');
      builder.push('</a>');

      $('#networks').append(builder.join('\n'));
    }).fail(function () {
      $('.alert').show();
      $('#submit').attr('disabled', '');
    });
  };

  var startNetworksWorker = function () {
    getAvailableNetworks();
    availableNetworksWorker = setInterval(getAvailableNetworks, 10 * 1000);
  };
  startNetworksWorker();

  // Handle network selection

  $(document).on('click', '#networks a', function (e) {
    e.preventDefault();

    $('#networks a').removeClass('active');
    $(this).addClass('active');

    if ($(this).attr('id') === 'hidden_network') {
      $('#wifi_ssid').attr('required', '');
      $('#form_wifi_ssid').show();
      hiddenNetwork = true;
    } else {
      $('#wifi_ssid').removeAttr('required');
      $('#form_wifi_ssid').hide();
      hiddenNetwork = false;
    }

    if ($(this).find('.glyphicon').text().trim() === 'Open') {
      $('#wifi_password').removeAttr('required');
      $('#form_wifi_password').hide();
      openNetwork = true;
    } else {
      $('#wifi_password').attr('required', '');
      $('#form_wifi_password').show();
      openNetwork = false;
    }
  });

  // Handle form submission

  $(document).on('submit', 'form', function (e) {
    e.preventDefault();
    $('#submit').attr('disabled', '');

    var wifi_ssid;
    if (hiddenNetwork) {
      wifi_ssid = $('#wifi_ssid').val();
    } else {
      wifi_ssid = $('#networks a.active h4').text();
    }
    var wifi_password;
    if (!openNetwork) {
      wifi_password = $('#wifi_password').val();
    } else {
      wifi_password = '';
    }
    var homie_host = $('#homie_host').val();
    var device_name = $('#device_name').val();

    var request = {
      wifi_ssid: wifi_ssid,
      wifi_password: wifi_password,
      homie_host: homie_host,
      name: device_name
    };

    $.ajax({
      url: url + '/config',
      method: 'PUT',
      data: JSON.stringify(request),
      contentType: 'application/json',
      dataType: 'json'
    }).done(function (data) {
      window.alert('Your device is configured. You can close this page.');
    }).fail(function () {
      $('.alert').show();
      $('#submit').removeAttr('disabled');
      window.alert('Configuration failed. Please retry.');
    });
  });
});
