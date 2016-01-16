'use strict';

import immutable from 'immutable';

import { createStore } from 'redux';

import request from 'request';

let homieRequest = request.defaults({
  baseUrl: 'http://homie.config',
  json: true
});

const SET_CONNECTION = 'SET_CONNECTION';
const SET_NETWORKS = 'SET_NETWORKS';
const SET_CONFIGURED = 'SET_CONFIGURED';
const SEND_NETWORKS_REQUEST = 'SEND_NETWORKS_REQUEST';
const SEND_CONFIG = 'SEND_CONFIG';

let immutableState = immutable.Map({
  networks: [], loading: false, connection: false, configured: false
});

function infrastructure (state = immutableState.toJS(), action) {
  switch (action.type) {
    case SET_CONNECTION:
      immutableState = immutableState.set('connection', action.connection);
      return immutableState.toJS();
    case SET_CONFIGURED:
      immutableState = immutableState.set('configured', true);
      return immutableState.toJS();
    case SET_NETWORKS:
      action.networks.sort(function (networkA, networkB) {
        if (networkA.rssi > networkB.rssi) {
          return -1;
        } else if (networkA.rssi < networkB.rssi) {
          return 1;
        } else {
          return 0;
        }
      });

      let networks = action.networks.map(function (network) {
        if (network.rssi <= -100) {
          network.signalQuality = 0;
        } else if (network.rssi >= -50) {
          network.signalQuality = 100;
        } else {
          network.signalQuality = 2 * (network.rssi + 100);
        }

        delete network.rssi;
        return network;
      });
      immutableState = immutableState.set('loading', false);
      immutableState = immutableState.set('networks', networks);
      return immutableState.toJS();
    case SEND_NETWORKS_REQUEST:
      immutableState = immutableState.set('loading', true);
      homieRequest.get('/networks', (err, res, body) => {
        if (err || res.statusCode !== 200) {
          store.dispatch({ type: SET_CONNECTION, connection: false });
        }

        store.dispatch({ type: SET_NETWORKS, networks: body.networks });
      });
      return state;
    case SEND_CONFIG:
      immutableState = immutableState.set('loading', true);
      homieRequest.put('/config', { body: action.config }, (err, res, body) => {
        if (err || res.statusCode !== 200) {
          store.dispatch({ type: SET_CONNECTION, connection: false });
        }

        store.dispatch({ type: SET_CONFIGURED });
        window.alert('Your Homie device is configured. You can close this page.');
      });
      return state;
    default:
      return state;
  }
}

let store = createStore(infrastructure);

let networksAsked = false;

setInterval(() => {
  homieRequest.get('/heart', (err, res, body) => {
    if (err || res.statusCode !== 200 || body.heart !== 'beat') {
      return store.dispatch({ type: SET_CONNECTION, connection: false });
    }

    store.dispatch({ type: SET_CONNECTION, connection: true });
    if (!networksAsked) {
      store.dispatch({ type: SEND_NETWORKS_REQUEST });
      networksAsked = true;
    }
  });
}, 5 * 1000);

export default store;

export function sendConfig (config) {
  return {
    type: SEND_CONFIG,
    config: config
  };
}
