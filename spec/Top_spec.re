open Jest;

Enzyme.configureEnzyme(Enzyme.react_16_adapter());

let testRepos =
    Node.Fs.readFileAsUtf8Sync ("spec/data/repos.json")
    |> Js.Json.parseExn
    |> Json.Decode.(array(Github.Decode.repository));

let testStargazers =
    Node.Fs.readFileAsUtf8Sync ("spec/data/stargazers.json")
    |> Js.Json.parseExn
    |> Json.Decode.(array(Github.Decode.stargazer));

let testClient: Top.client = {
    fetchOrgRepositories: _ => Js.Promise.resolve(Either.Right((None, testRepos))),
    fetchStargazers: _ => Js.Promise.resolve(Either.Right((None, testStargazers)))
}

let setup = (~orgname="facebook", ~client=testClient, ()) =>
  EnzymeExtra.mount(<Top orgname client />);



describe("Top", () => {
  open Expect;

  test("loads repositories when search is clicked", () => {
    let wrapper = setup();
    wrapper
        |> Enzyme.find(".search")
        |> Enzyme.simulate("click");
    let state: Top.state = Enzyme.state(wrapper);
    expect(state.repositories) |> toBe(Some(testRepos))
    });
});