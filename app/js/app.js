'use strict';

import React from 'react';
import ReactDOM from 'react-dom';
import classNames from 'classnames';

import { Provider, connect } from 'react-redux';
import store, {sendConfig} from './store';

@connect(state => ({
  networks: state.networks,
  loading: state.loading,
  connection: state.connection,
  configured: state.configured
}), (dispatch) => ({
  sendConfig: (config) => dispatch(sendConfig(config))
}))
class App extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      selectedNetworkId: null,
      selectedSsid: null
    };
  }

  onNetworkClick (e, id, ssid) {
    e.preventDefault();
    this.setState({ selectedNetworkId: id, selectedSsid: ssid });
  }

  onFormSubmit (e) {
    e.preventDefault();

    let ssid;
    if (this.state.selectedNetworkId === 'hidden') {
      ssid = this.refs.ssidInput.value;
    } else {
      ssid = this.state.selectedSsid;
    }
    let password = this.refs.passwordInput.value;
    let host = this.refs.hostInput.value;
    let name = this.refs.nameInput.value;

    store.dispatch(sendConfig({
      name: name,
      wifi_ssid: ssid,
      wifi_password: password,
      homie_host: host
    }));
  }

  render () {
    let errorMessage;
    if (!this.props.connection) {
      errorMessage = <div className='alert alert-danger' role='alert'>
        <span className='glyphicon glyphicon-exclamation-sign' aria-hidden='true'></span> <span className='sr-only'>Error:</span>
        Cannot connect to the device. Are you sure you are connected to its Wi-Fi?
      </div>;
    }

    let networks = this.props.networks.map((network, index) => {
      let progressbarClasses = classNames({
        'progress-bar': true,
        'progress-bar-success': network.signalQuality >= 66,
        'progress-bar-warning': network.signalQuality < 66 && network.signalQuality >= 33,
        'progress-bar-danger': network.signalQuality < 33
      });

      let encryption;
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

      let aClasses = classNames({
        'list-group-item': true,
        active: this.state.selectedNetworkId === index
      });

      return (
        <a href='#' className={aClasses} key={index} onClick={(e) => this.onNetworkClick(e, index, network.ssid)}>
            <h4 className='list-group-item-heading'>{ network.ssid }</h4>
            <p className='list-group-item-text'>
                <div className='progress'>
                  <div className={ progressbarClasses } role='progressbar' aria-valuenow={ network.signalQuality } aria-valuemin='0' aria-valuemax='100' style={{ minWidth: '6em', width: `${network.signalQuality}%` }}>
                    Signal: { network.signalQuality }%
                  </div>
                </div>
                <span className='glyphicon glyphicon-lock'> { encryption }</span>
            </p>
        </a>
      );
    });

    let aClasses = classNames({
      'list-group-item': true,
      active: this.state.selectedNetworkId === 'hidden'
    });

    networks.push(<a href='#' className={aClasses} id='hidden_network' key='hidden' onClick={(e) => this.onNetworkClick(e, 'hidden', null)}>
      <h4 className='list-group-item-heading'>Hidden network</h4>
      <p className='list-group-item-text'>
        <p>Your network might be configured to not advertise itself. Select this option if this is the case.</p>
      </p>
    </a>);

    let ssidForm;
    if (this.state.selectedNetworkId === 'hidden') {
      ssidForm = <div className='form-group' id='form_wifi_ssid'>
        <label htmlFor='wifi_ssid'>Wi-Fi SSID</label>
        <input type='text' className='form-control' id='wifi_ssid' placeholder='SSID' maxLength='32' ref='ssidInput' required/>
        <p className='help-block'>Maximum of 32 characters</p>
      </div>;
    }

    return (
      <div className='container'>
        { errorMessage }

        <div className='page-header'>
          <h1>Connect your device</h1>
          <p className='lead'>Your device needs to be connected to its server to work correctly.</p>
        </div>

        <p>First, we need to connect to your home Wi-Fi. Select your network in the list below.</p>

        <div className='list-group' id='networks'>
          <b>{ networks }</b>
        </div>

        <form onSubmit={ (e) => this.onFormSubmit(e) }>
          {ssidForm}

          <div className='form-group' id='form_wifi_password'>
            <label htmlFor='wifi_password'>Wi-Fi password</label>
            <input type='password' className='form-control' id='wifi_password' placeholder='Password' maxLength='63' ref='passwordInput' required/>
            <p className='help-block'>Maximum of 63 characters</p>
          </div>

          <hr/>

          <p>Next, we need to know where on your network is located the master server.</p>

          <div className='form-group'>
            <label htmlFor='homie_host'>Homie host</label>
            <input type='text' className='form-control' id='homie_host' placeholder='Host' maxLength='63' ref='hostInput' required/>
            <p className='help-block'>Maximum of 63 characters</p>
          </div>

          <hr/>

          <p>Finally, name your device.</p>

          <div className='form-group'>
            <label htmlFor='device_name'>Device name</label>
            <input type='text' className='form-control' id='device_name' placeholder='Name' maxLength='63' pattern='^[^\-][a-z0-9\-]+[^\-]$' ref='nameInput' required/>
            <p className='help-block'>Lowercase alphanumeric with optional dashes. Must not start or end with a dash.<br/>
            Maximum of 32 characters. The name must be unique on your Homie network</p>
          </div>

          <p>By clicking on <span className='label label-primary'>Submit</span> below, your device will reboot and the red LED will start blinking. If everything was configured correctly, it should stop blinking within a minute. If it continues to blink slowly (every second), there is a problem with Wi-Fi credentials. If it blinks faster (every 0.2 second), there is a problem with MQTT credentials. You can reset the device anytime to reconfigure, by pressing the <span className='label label-default'>FLASH</span> button for a dozen of seconds until the LED stays on.</p>

          <button type='submit' className='btn btn-primary' id='submit' disabled={ this.state.selectedNetworkId === null || this.props.loading || !this.props.connection || this.props.configured }>Submit</button>
        </form>
      </div>
    );
  }
}

ReactDOM.render(
  <Provider store={store}>
    <App/>
  </Provider>,
  document.getElementById('app')
);

App.propTypes = {
  loading: React.PropTypes.bool.isRequired,
  networks: React.PropTypes.array.isRequired,
  connection: React.PropTypes.bool.isRequired,
  configured: React.PropTypes.bool.isRequired,
  sendConfig: React.PropTypes.func.isRequired
};
