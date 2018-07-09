/* We can only require styles safely in this module since the test bundler bypasses webpack */
[%%raw "require('style/index.scss')"];


ReactDOMRe.renderToElementWithId(<Main orgname="facebook" />, "app");