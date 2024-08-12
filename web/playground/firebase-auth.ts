// @ts-nocheck

import { initializeApp } from 'firebase/app';
import { getAuth, signInWithPopup, GithubAuthProvider, UserInfo } from 'firebase/auth';
import * as kv from 'idb-keyval';

export class FirebaseAuth {
  constructor() {
    /** @type {?string} */
    this._accessToken = null;
    this._firebaseApp = initializeApp({
      apiKey: "AIzaSyCFjQIg_wTeffzEapwrSIi3sxvCA_-VSH8",
      authDomain: "zquestclassic.firebaseapp.com",
      projectId: "zquestclassic",
      storageBucket: "zquestclassic.appspot.com",
      messagingSenderId: "896304980086",
      appId: "1:896304980086:web:245f14df944cfdbc993f57",
      measurementId: "G-ZMZF2YNV4M"
    });
    this._auth = getAuth();
    this._provider = new GithubAuthProvider();
    this._provider.addScope('gist');

    /**
     * Promise which resolves after the first check of an existing access token.
     * @type {Promise<void>}
     */
    this._ready = Promise.resolve(
      kv.get('accessToken').then((token) => {
        if (token) {
          this._accessToken = token;
        }
      })
    ).then(() => Promise.resolve(
      kv.get('githubUser').then((user) => {
        if (user) {
          this._githubUser = user;
        }
      })
    ));
  }

  /**
   * Returns the GitHub access token if already logged in. If not logged in,
   * returns null (and will not trigger sign in).
   * @return {Promise<?string>}
   */
  async getAccessTokenIfLoggedIn() {
    await this._ready;
    return this._accessToken;
  }

  async getGithubUserIfLoggedIn(): Promise<UserInfo> {
    await this._ready;
    return this._githubUser;
  }

  /**
   * Returns the GitHub access token, triggering sign in if needed.
   * @return {Promise<string>}
   */
  async getAccessToken() {
    await this._ready;
    if (this._accessToken) return this._accessToken;
    return this.signIn();
  }

  async getGithubUser(): Promise<UserInfo> {
    await this._ready;
    if (!this._githubUser) await this.signIn();
    return this._githubUser;
  }

  /**
   * Signs in the user to GitHub using the Firebase API.
   * @return {Promise<string>} accessToken
   */
  async signIn() {
    const result = await signInWithPopup(this._auth, this._provider);
    const credential = GithubAuthProvider.credentialFromResult(result);
    if (!credential || !credential.accessToken) throw new Error('unexpected credential');

    const accessToken = credential.accessToken;
    this._accessToken = accessToken;
    // A limitation of firebase auth is that it doesn't return an oauth token
    // after a page refresh: `onAuthStateChanged` returns a firebase user, which has no knowledge
    // of GitHub's oauth token. Since GitHub's tokens never expire, stash the access token in IDB.
    await kv.set('accessToken', accessToken);

    this._githubUser = result.user.providerData[0];
    await kv.set('githubUser', this._githubUser);

    return accessToken;
  }
}
