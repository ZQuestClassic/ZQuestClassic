import { configureMount, getAttachedDirHandle } from "./settings.js";

function fail(message) {
  alert(message);
  window.location.reload();
}

export async function handleFileLaunch() {
  if (!(window.launchQueue && 'files' in LaunchParams.prototype)) {
    document.body.textContent = 'This page is only for opening files from an installed PWA';
    return;
  }

  await configureMount();
  const dirHandle = getAttachedDirHandle();
  if (!dirHandle) {
    fail('Sorry, you can only open files this way if you\'ve attached a folder. See Settings');
    return;
  }

  document.querySelector('.content').classList.add('hidden');
  document.querySelector('.file-handler').classList.remove('hidden');

  const launchedFile = await new Promise((resolve, reject) => {
    launchQueue.setConsumer(async (launchParams) => {
      console.log('...', launchParams.files);
      if (!launchParams.files.length) {
        reject(new Error('No files found'));
        return;
      }

      resolve(launchParams.files[0]);
    });
  });

  const relativePaths = await dirHandle.resolve(launchedFile);
  if (!relativePaths) {
    fail('Sorry, but that file is not inside the attached folder and cannot be opened directly');
    return;
  }

  const relativePath = relativePaths.join('/');
  const quest = `local/${relativePath}`;

  const openInEditor = await new Promise(resolve => {
    document.querySelectorAll('.file-handler button')[0].addEventListener('click', () => {
      resolve(false);
    });
    document.querySelectorAll('.file-handler button')[1].addEventListener('click', () => {
      resolve(true);
    });
  });

  document.querySelector('.content').classList.remove('hidden');
  document.querySelector('.file-handler').classList.add('hidden');

  return {openInEditor, quest};
}
