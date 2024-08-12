export class Logger {
  #el: HTMLElement;
  #id?: NodeJS.Timeout;

  constructor(element: HTMLElement) {
    this.#el = element;
    this.#el.id = 'zc-log';
    const styleEl = document.createElement('style');
    styleEl.textContent = /* css */ `
		#zc-log {
			position: fixed;
			background-color: #323232;
			color: #fff;
			min-height: 48px;
			min-width: 288px;
			box-shadow: 0 2px 5px 0 rgba(0, 0, 0, 0.26);
			border-radius: 2px;
			margin: 12px;
			font-size: 14px;
			cursor: default;
			transition: transform 0.3s, opacity 0.3s;
			transform: translateY(100px);
			opacity: 0;
			bottom: 0;
			left: 0;
			z-index: 3;
			display: flex;
			flex-direction: row;
			justify-content: center;
			align-items: center;
		}
		
		#zc-log.zc-show {
			opacity: 1;
			transform: translateY(0);
		}
		`;
    if (!this.#el.parentNode) throw new Error('element needs to be in the DOM');
    this.#el.parentNode.insertBefore(styleEl, this.#el);
    this.#id = undefined;
  }

  log(msg: string, autoHide = true) {
    this.#id && clearTimeout(this.#id);

    this.#el.textContent = msg;
    this.#el.classList.add('zc-show');
    if (autoHide) {
      this.#id = setTimeout(() => {
        this.#el.classList.remove('zc-show');
      }, 7000);
    }
  }

  warn(msg: string) {
    this.log('Warning: ' + msg);
  }

  error(msg: string) {
    this.log(msg);
    setTimeout(() => {
      throw new Error(msg);
    }, 0);
  }

  hide() {
    this.#id && clearTimeout(this.#id);
    this.#el.classList.remove('zc-show');
  }
}
