'use strict';

import React from 'react';
import ReactDOM from 'react-dom';

const BASE_API = 'http://homie.config';

const STEP_CONNECTION = 1;
const STEP_INFO = 2;
const STEP_WIFI = 3;
const STEP_MQTT = 4;
const STEP_DETAILS = 5;
const STEP_SENDING = 6;

class App extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      step: STEP_CONNECTION,
      name: null,
      wifi: { },
      mqtt: { },
      ota: { }
    };
  }

  setStep (step) {
    this.setState({ step });
  }

  setName (name) {
    this.setState({ name });
  }

  setWifiCreds (creds) {
    this.setState({ wifi: creds });
  }

  setMqttCreds (creds) {
    this.setState({ mqtt: creds });
  }

  setOtaCreds (creds) {
    this.setState({ ota: creds });
  }

  sendConfig () {
    let body = {
      name: this.state.name,
      wifi: this.state.wifi,
      mqtt: this.state.mqtt,
      ota: this.state.ota
    };

    let options = {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(body)
    };

    return new Promise((resolve, reject) => {
      window.fetch(`${BASE_API}/config`, options).then((res) => {
        return res.json();
      }).then((json) => {
        if (json.success) {
          resolve();
        } else {
          reject();
        }
      }).catch(() => {
        reject();
      });
    });
  }

  render () {
    let Step;

    switch (this.state.step) {
      case STEP_CONNECTION:
        Step = ConnectionStep;
        break;
      case STEP_INFO:
        Step = InfoStep;
        break;
      case STEP_WIFI:
        Step = WifiStep;
        break;
      case STEP_MQTT:
        Step = MqttStep;
        break;
      case STEP_DETAILS:
        Step = DetailsStep;
        break;
      case STEP_SENDING:
        Step = SendingStep;
        break;
    }

    return (
      <div className='container'>
        <h1 className='title'>Homie for ESP8266 configurator</h1>
        <h2 className='subtitle'>Configure your device.</h2>

        <div className='tabs is-centered'>
          <ul>
            <li className={this.state.step === STEP_CONNECTION ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-heartbeat'></i> Connection</a></li>
            <li className={this.state.step === STEP_INFO ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-info'></i> Information</a></li>
            <li className={this.state.step === STEP_WIFI ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-wifi'></i> Wi-Fi</a></li>
            <li className={this.state.step === STEP_MQTT ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-signal'></i> MQTT</a></li>
            <li className={this.state.step === STEP_DETAILS ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-cogs'></i> Details</a></li>
            <li className={this.state.step === STEP_SENDING ? 'is-active' : ''}><a href='' onClick={(e) => { e.preventDefault(); } }><i className='fa fa-rocket'></i> Sending</a></li>
          </ul>
        </div>

        <Step setStep={(step) => this.setStep(step)} setWifiCreds={(creds) => this.setWifiCreds(creds)} setMqttCreds={(creds) => this.setMqttCreds(creds)} setName={(name) => this.setName(name)} setOtaCreds={(creds) => this.setOtaCreds(creds)} sendConfig={() => this.sendConfig()} />
      </div>
    );
  }
}

class ConnectionStep extends React.Component {
  constructor (props) {
    super(props);
  }

  componentDidMount () {
    let interval;
    let heartbeat = () => {
      window.fetch(`${BASE_API}/heart`).then((res) => {
        if (res.ok) {
          window.clearInterval(interval);
          this.props.setStep(STEP_INFO);
        }
      });
    };

    interval = window.setInterval(heartbeat, 5 * 1000);
    heartbeat();
  }

  render () {
    return (
      <div>
        <div className='content'>
          <p>
            Connect to your device Wi-Fi AP.
            If the AP is named <span className='tag is-dark'>Homie-1234abcd</span>,
            the password is <span className='tag is-dark'>1234abcd</span>.
          </p>
        </div>
        <div className='notification is-info'>
          <span className='button is-info is-loading'>Loading</span>
          Waiting for the device...
        </div>
      </div>
    );
  }
}
ConnectionStep.propTypes = {
  setStep: React.PropTypes.func.isRequired
};

class InfoStep extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      loading: true,
      info: {}
    };
  }

  componentDidMount () {
    let interval;
    let deviceinfo = () => {
      console.log('Triggering request');
      window.fetch(`${BASE_API}/device-info`).then((res) => {
        if (res.ok) {
          window.clearInterval(interval);
          return res.json();
        }
      }).then((json) => {
        this.setState({
          loading: false,
          info: json
        });
      });
    };

    interval = window.setInterval(deviceinfo, 5 * 1000);
    deviceinfo();
  }

  handleNextButton (e) {
    e.preventDefault();

    this.props.setStep(STEP_WIFI);
  }

  render () {
    return (
      <div>
        {(() => {
          if (this.state.loading) {
            return (
              <div className='notification is-info'>
                <span className='button is-info is-loading'>Loading</span>
                Gathering device information...
              </div>
            );
          } else {
            return (
              <div className='content'>
                <p>
                  Here are some information about the device you are about to configure:
                </p>

                <ul>
                  <li><span className='icon is-small'><i className='fa fa-hashtag'></i></span> Device ID: { this.state.info.device_id }</li>
                  <li><span className='icon is-small'><i className='fa fa-tag'></i></span> Homie version: { this.state.info.homie_version }</li>
                  <li><span className='icon is-small'><i className='fa fa-font'></i></span> Firmware name: { this.state.info.firmware.name }</li>
                  <li><span className='icon is-small'><i className='fa fa-tag'></i></span> Firmware version: { this.state.info.firmware.version }</li>
                  <li>
                    <span className='icon is-small'><i className='fa fa-cubes'></i></span> Nodes:
                    <table className='table'>
                      <thead>
                        <tr>
                          <th>ID</th>
                          <th>Type</th>
                        </tr>
                      </thead>
                      <tbody>
                        { this.state.info.nodes.map((node, i) => {
                          return (
                            <tr key={i}>
                              <td>{ node.id }</td>
                              <td>{ node.type }</td>
                            </tr>
                          );
                        }) }
                      </tbody>
                    </table>
                  </li>
                </ul>

                <a className='button is-primary' onClick={(e) => { this.handleNextButton(e); } }>Next</a>
              </div>
            );
          }
        })()}
      </div>
    );
  }
}
InfoStep.propTypes = {
  setStep: React.PropTypes.func.isRequired
};

class WifiStep extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      loading: true,
      networks: {},
      buttonDisabled: true,
      selectedSsid: null,
      showSsidInput: false,
      showPasswordInput: false
    };
  }

  componentDidMount () {
    let interval;
    let networks = () => {
      window.fetch(`${BASE_API}/networks`).then((res) => {
        if (res.ok) {
          window.clearInterval(interval);
          return res.json();
        }
      }).then((json) => {
        this.setState({
          loading: false,
          networks: json
        });
      });
    };

    interval = window.setInterval(networks, 5 * 1000);
    networks();
  }

  handleSelectChange (e) {
    if (e.target.value === 'select') {
      this.setState({ showSsidInput: false, showPasswordInput: false, selectedSsid: null, buttonDisabled: true });
    } else if (e.target.value === 'other') {
      this.setState({ showSsidInput: true, showPasswordInput: true, selectedSsid: null, buttonDisabled: false });
    } else {
      let data = e.target.options[e.target.selectedIndex].dataset;
      this.setState({ showSsidInput: false, showPasswordInput: data.open === 'no', selectedSsid: data.ssid, buttonDisabled: false });
    }
  }

  handleFormSubmit (e) {
    e.preventDefault();

    let creds = {};

    if (this.state.selectedSsid) {
      creds.ssid = this.state.selectedSsid;
    } else {
      creds.ssid = this.refs.ssid.value;
    }
    creds.password = this.refs.password.value;

    this.props.setWifiCreds(creds);
    this.props.setStep(STEP_MQTT);
  }

  render () {
    return (
      <div>
        {(() => {
          if (this.state.loading) {
            return (
              <div className='notification is-info'>
                <span className='button is-info is-loading'>Loading</span>
                Gathering available networks...
              </div>
            );
          } else {
            this.state.networks.networks.sort(function (networkA, networkB) {
              if (networkA.rssi > networkB.rssi) {
                return -1;
              } else if (networkA.rssi < networkB.rssi) {
                return 1;
              } else {
                return 0;
              }
            });

            let networks = this.state.networks.networks.map(function (network) {
              if (network.rssi <= -100) {
                network.signalQuality = 0;
              } else if (network.rssi >= -50) {
                network.signalQuality = 100;
              } else {
                network.signalQuality = 2 * (network.rssi + 100);
              }

              switch (network.encryption) {
                case 'wep':
                  network.encryption = 'WEP';
                  break;
                case 'wpa':
                  network.encryption = 'WPA';
                  break;
                case 'wpa2':
                  network.encryption = 'WPA2';
                  break;
                case 'none':
                  network.encryption = 'Open';
                  break;
                case 'auto':
                  network.encryption = 'Automatic';
                  break;
              }
              return network;
            });

            return (
              <div className='content'>
                <p>
                  Select the Wi-Fi network to connect to:
                </p>

                <form onSubmit={ (e) => this.handleFormSubmit(e) }>
                  <p className='control'>
                    <span className='select'>
                      <select ref='select' onChange={ (e) => { this.handleSelectChange(e); } }>
                        <option value='select' key='select'>Select a network...</option>
                        { networks.map((network, i) => {
                          return (
                            <option value={i} data-ssid={ network.ssid } data-open={ network.encryption === 'Open' ? 'yes' : 'no' } onSelect={ (e) => { window.alert(network.ssid); } } key={i}>
                              { network.ssid } - { network.encryption } ({ network.signalQuality }%)
                            </option>
                          );
                        }) }

                        <option value='other' key='other'>Other/Hidden network</option>
                      </select>
                    </span>
                  </p>

                  {(() => {
                    if (this.state.showSsidInput) {
                      return (
                        <p className='control'>
                          <input ref='ssid' className='input' type='text' placeholder='Network SSID' maxLength='32' required />
                        </p>
                      );
                    }
                  })()}

                  {(() => {
                    if (this.state.showPasswordInput) {
                      return (
                        <p className='control'>
                          <input ref='password' className='input' type='password' placeholder='Network password (leave blank if open network)' />
                        </p>
                      );
                    }
                  })()}

                  <p className='control'>
                    <button type='submit' disabled={ this.state.buttonDisabled } className='button is-primary'>Next</button>
                  </p>
                </form>
              </div>
            );
          }
        })()}
      </div>
    );
  }
}
WifiStep.propTypes = {
  setStep: React.PropTypes.func.isRequired,
  setWifiCreds: React.PropTypes.func.isRequired
};

class MqttStep extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      showLoginInput: false
    };
  }

  handleCheckboxChange (e) {
    if (e.target.value === 'select') {
      this.setState({ showSsidInput: false, showPasswordInput: false, selectedSsid: null, buttonDisabled: true });
    } else if (e.target.value === 'other') {
      this.setState({ showSsidInput: true, showPasswordInput: true, selectedSsid: null, buttonDisabled: false });
    } else {
      let data = e.target.options[e.target.selectedIndex].dataset;
      this.setState({ showSsidInput: false, showPasswordInput: data.open === 'no', selectedSsid: data.ssid, buttonDisabled: false });
    }
  }

  handleAuthChange (e) {
    this.setState({ showLoginInput: e.target.checked });
  }

  handleFormSubmit (e) {
    e.preventDefault();

    let creds = {};
    creds.host = this.refs.host.value;
    creds.port = parseInt(this.refs.port.value, 10);

    creds.ssl = false;

    creds.auth = false;

    if (this.state.showLoginInput) {
      creds.auth = true;
      creds.username = this.refs.username.value;
      creds.password = this.refs.password.value;
    }

    this.props.setMqttCreds(creds);
    this.props.setStep(STEP_DETAILS);
  }

  render () {
    return (
      <div className='content'>
        <p>
          Enter the MQTT credentials.
        </p>

        <form onSubmit={ (e) => this.handleFormSubmit(e) }>
          <p className='control'>
            <input ref='host' className='input' type='text' placeholder='MQTT broker host' required />
          </p>

          <p className='control'>
            <input ref='port' className='input' type='number' step='1' defaultValue='1883' min='1' max='65535' placeholder='MQTT broker port' required />
          </p>

          <p className='control'>
            <label className='checkbox'>
              <input ref='auth' type='checkbox' onChange={ (e) => this.handleAuthChange(e) } />
              Use MQTT authentication
            </label>
          </p>

          {(() => {
            if (this.state.showLoginInput) {
              return (
                <div>
                  <p className='control'>
                    <input ref='username' className='input' type='text' placeholder='MQTT username' required />
                  </p>

                  <p className='control'>
                    <input ref='password' className='input' type='password' placeholder='MQTT password' required />
                  </p>
                </div>
              );
            }
          })()}

          <p className='control'>
            <button type='submit' disabled={ this.state.buttonDisabled } className='button is-primary'>Next</button>
          </p>
        </form>
      </div>
    );
  }
}
MqttStep.propTypes = {
  setStep: React.PropTypes.func.isRequired,
  setMqttCreds: React.PropTypes.func.isRequired
};

class DetailsStep extends React.Component {
  constructor (props) {
    super(props);
  }

  handleFormSubmit (e) {
    e.preventDefault();

    this.props.setName(this.refs.name.value);
    this.props.setOtaCreds({ enabled: this.refs.ota.checked });
    this.props.setStep(STEP_SENDING);
  }

  render () {
    return (
      <div className='content'>
        <p>
          A few details before finishing the configuration.
        </p>

        <form onSubmit={ (e) => this.handleFormSubmit(e) }>
          <p className='control'>
            <input ref='name' className='input' type='text' placeholder='Friendly name of the device' required />
          </p>

          <p className='control'>
            <label className='checkbox'>
              <input ref='ota' type='checkbox' />
              Enable OTA
            </label>
          </p>

          <p className='control'>
            <button type='submit' className='button is-primary'>Next</button>
          </p>
        </form>
      </div>
    );
  }
}
DetailsStep.propTypes = {
  setStep: React.PropTypes.func.isRequired,
  setName: React.PropTypes.func.isRequired,
  setOtaCreds: React.PropTypes.func.isRequired
};

class SendingStep extends React.Component {
  constructor (props) {
    super(props);
    this.state = {
      loading: true,
      success: null
    };
  }

  componentDidMount () {
    this.props.sendConfig().then(() => {
      this.setState({ loading: false, success: true });
    }).catch(() => {
      this.setState({ loading: false, success: false });
    });
  }

  render () {
    return (
      <div>
        {(() => {
          if (this.state.loading) {
            return (
              <div className='notification is-info'>
                <span className='button is-info is-loading'>Loading</span>
                Sending configuration...
              </div>
            );
          } else {
            if (this.state.success) {
              return (
                <div className='notification is-success'>
                  The configuration was sent. You can close this page.
                </div>
              );
            } else {
              return (
                <div className='notification is-danger'>
                  There was an error while sending the configuration. Please retry.
                </div>
              );
            }
          }
        })()}
      </div>
    );
  }
}
SendingStep.propTypes = {
  sendConfig: React.PropTypes.func.isRequired
};

ReactDOM.render(<App/>, document.getElementById('app'));
