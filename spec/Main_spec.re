open Jest;
open Github;
open FetchResponse;

Enzyme.configureEnzyme(Enzyme.react_16_adapter());

/* The fact that loading fixtures doesn't throw as a module-import
 * side-effect tests that the json decoders work
 */
let repoFixture =
    Node.Fs.readFileAsUtf8Sync ("spec/data/repos.json")
    |> Js.Json.parseExn
    |> Json.Decode.(array(Decode.repository));

let stargazersFixture =
    Node.Fs.readFileAsUtf8Sync ("spec/data/stargazers.json")
    |> Js.Json.parseExn
    |> Json.Decode.(array(Decode.stargazer));

/* a dummy client we inject as a mock to test the reducer */
let testClient: client = {
    fetchOrgRepositories: _ => Js.Promise.resolve(Success(repoFixture,None)),
    fetchStargazers: _ => Js.Promise.resolve(Success(stargazersFixture,None))
}

let setup = (~orgname="facebook", ~maxStargazers=20, ~client=testClient, ()) =>
    /* We use `mount` instead of `shallow` so we can find nodes
       in the subcomponents Main coordinates */
    EnzymeExtra.mount(<Main orgname client maxStargazers />);



describe("Main", () => {
  open Expect;

  /***
   * I'd like to write this test (and more "integration-like" tests like this one)
   * but the `Enzyme.state` retrieved after simulating the click (or anything which
   * triggers a signal emission) seems to be the initial one, not the one expected
   * after an application of the reducer.
   * 
   * After some "leave a bomb, let's here see if it goes off" debugging I've confirmed
   * that the reducer is actually called after the event handlers send their signals, it's
   * just that it's not the internal `reasonState` that `Enzyme.state` retrieves is not
   * the "reduced" one.
   * 
   * Could it be because the `Update`/`UpdateWithSideEffects` commands are not being evaluated
   * in the test environment due to the way ReasonReact's custom createClass works?
   * 
   * I've exhausted my google-fu looking for the recommended way to unit-test reason-react
   * components with no avail. Closest thing I've found is this (as today, unanswered)
   * question on the forums: https://reasonml.chat/t/testing-reason-react/520. Studying
   * ReasonReact's source (up to my limited understanding of the language) didn't give me
   * an answer either as to how to implement a work-around.
   * 
   * So I'll try another strategy instead:
   * 
   *  - Test rendering logic as a function of (state, props) by instantiating the component
   *    with suitable props and modifying instrusively the component-under-test's state
   *    so we can verify the resulting vdom is correct.
   * 
   *  - Test the reducer in isolation to verify that it updates the state in the expected
   *    way in response to actions/signals. Also test (by injecting mocks) that it produces
   *    the desired side-effects (like calling the mocked-up remote API client)
   * 
   * This should offer pretty good coverage, leaving only the event-handlers which emit the signals
   * untested. We should limit the damage by keeping these very small.

  test("loads repositories when search is clicked", () => {
    let wrapper = setup();
    wrapper
        |> Enzyme.find(".search")
        |> Enzyme.simulate("click");
    let state: Main.state = Enzyme.state(wrapper);
    expect(state.repositories) |> toBe(Some(repoFixture))
    });
    */
  describe("rendering", () => {

    test("shows orgname in header", () => {
      let orgname = "acmeinc";
      let wrapper = setup(~orgname, ());
      let text = wrapper
          |> Enzyme.find(".repositories.column .header")
          |> Enzyme.text;
      expect(text) |> toContainString(orgname);
      });

    test("uses orgname as inital value of search field", () => {
      let orgname = "acmeinc";
      let wrapper = setup(~orgname, ());
      let text = wrapper
          |> Enzyme.find(".ui.icon.input input")
          |> Enzyme.html;
      expect(text) |> toContainString("value=\""++orgname++"\"");
      });

    test("doesn't show a .link.more node when there's nothing more to load", () => {
      let wrapper = setup();
      let nodes = wrapper |> Enzyme.find(".link.more");
      expect(Enzyme.length(nodes)) |> toBe(0);
    });

    test("shows a .link.more node when there's more to load", () => {
      let wrapper = setup();
      wrapper
        |. EnzymeExtra.modifyState(s:Main.state => {...s, repositories:Some(Success([||], Some("foo")))});
      let nodes = wrapper |> Enzyme.find(".link.more");
      expect(Enzyme.length(nodes)) |> toBe(1);
    });

    test("doesn't show repositories when there's none loaded", () => {
      let wrapper = setup();
      let nodes = wrapper |> Enzyme.find(".repository");
      expect(Enzyme.length(nodes)) |> toBe(0);
    });

    test("shows repositories when loaded", () => {
      let wrapper = setup();
      wrapper
        |. EnzymeExtra.modifyState(s:Main.state =>
            {...s, repositories:Some(Success(repoFixture,None))}
            );
      let nodes = wrapper |> Enzyme.find(".repository");
      expect(Enzyme.length(nodes)) |> toBe(30);
    });

    test("doesn't show stargazers when there's none loaded", () => {
      let wrapper = setup();
      let nodes = wrapper |> Enzyme.find(".stargazer");
      expect(Enzyme.length(nodes)) |> toBe(0);
    });

    test("shows up to maxStargazers stargazers when loaded", () => {
      let maxStargazers = 7;
      let wrapper = setup(~maxStargazers, ());
      wrapper
        |. EnzymeExtra.modifyState(s:Main.state =>
            {...s, stargazers:Some(Success(stargazersFixture,None))}
            );
      let nodes = wrapper |> Enzyme.find(".stargazer");
      expect(Enzyme.length(nodes)) |> toBeLessThanOrEqual(maxStargazers);
    });

    test("shows repo's fullName in stargazers column when a repo is selected", () => {
      let wrapper = setup();
      let repo = repoFixture[0];
      wrapper
        |. EnzymeExtra.modifyState(s:Main.state =>
            {...s, selectedRepo:Some(repo)}
            );
      let text = wrapper
        |> Enzyme.find(".stargazers.column .header")
        |> Enzyme.text;
      expect(text) |> toContainString(repo.fullName);
    });

    testPromise("displays error message when there's an error when communicating with the API", () => {
      open Js.Promise;
      let wrapper = setup();
      make((~resolve, ~reject) =>
        /* convoluted way of using `resolve` to avoid a warning. Is there a better way? */
        if (false) {
          resolve(. expect(true)|>toBe(false))
        } else {
          reject(. Not_found)
        })
        |> catch(error => {
            wrapper
              |. EnzymeExtra.modifyState(s:Main.state =>
                  {...s, repositories:Some(NetworkError(error))}
                  );
            let nodes = wrapper
              |> Enzyme.find(".error.message");
            resolve(expect(Enzyme.length(nodes)) |> toBe(1));
        });
    });

  });

});