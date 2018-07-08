open Github;

type action =
  | Init
  | GetRepositories(bool)
  | GotRepositories(bool, fetchResponse(array(repository)))
  | SetSelected(repository)
  | SetOrgname(string)
  ;

type state = {
    orgname: string,
    repositories: option(array(repository)),
    selectedRepo: option(repository),
    nextPageUrl: option(string),
    lastError: option(Js.Promise.error),
    loading: bool
};

type client = {
    fetchOrgRepositories: getRepositories,
    fetchStargazers: getStargazers
}

let defaultClient = {
    fetchOrgRepositories: fetchOrgRepositories,
    fetchStargazers: fetchStargazers
};

let component = ReasonReact.reducerComponent("ReGitHubStargazers");

let make = (~orgname, ~client=defaultClient, _children) => {
    ...component,

    initialState: () => {
        orgname,
        repositories: None,
        selectedRepo: None,
        nextPageUrl: None,
        lastError: None,
        loading: false
    },

    didMount: self => self.send(Init),

    reducer: action => {
        let getRepositories = self => self . ReasonReact.send(GetRepositories(false));
        switch (action) {
        | Init => (_state => ReasonReact.SideEffects(getRepositories));
        
        | GetRepositories(append) => (state =>
            ReasonReact.UpdateWithSideEffects({...state, lastError:None, loading:true}, self => {
                if (String.length(state.orgname) > 0) {
                    let url = Js.Option.getWithDefault (orgReposUrl(state.orgname)) (state.nextPageUrl);
                    Js.Promise.(
                        client.fetchOrgRepositories(url)
                        |> then_(result => self.send(GotRepositories(append, result)) |> resolve)
                        |> ignore
                    )
                }
            }))

        | GotRepositories(append, result) => (state =>
            switch(result) {
            | Right((nextPageUrl,repos)) => {
                let repositories =
                    if (append) {
                        switch(state.repositories) {
                        | Some(curRepos) => Some(Array.append(curRepos, repos));
                        | None           => Some(repos);
                        }
                    } else Some(repos);
                ReasonReact.Update({...state, loading:false, repositories, nextPageUrl});
                }
            | Left(err) => ReasonReact.Update({...state, loading:false, lastError: Some(err)});
            });

        | SetOrgname(orgname) => (state =>
            ReasonReact.UpdateWithSideEffects({...state, orgname, nextPageUrl:None}, getRepositories));

        | SetSelected(repo) => (state =>
            ReasonReact.Update({...state, selectedRepo:Some(repo)}));
        }
    },

    render: ({state,send}) => {
        open ReasonReact;
        <div className="ui container">
            <TextInput value=state.orgname onChange=(orgname => (send(SetOrgname(orgname)))) />
            <div className="ui segment">
                <div className="ui two column stackable grid layout">

                    <div className="column">
                        <h2 className="header">(string(state.orgname))</h2>
                        <div className="ui list">
                            (switch(state.repositories) {
                                | Some(repos) => array(Array.map((repository =>
                                    <Repository
                                        repository
                                        onStargazersClick=(_ => send(SetSelected(repository)))
                                        />
                                    ), repos))
                                | None => null
                            })
                        </div>
                        (switch (state.nextPageUrl) {
                        | Some(_) =>
                            <a href="#"
                               onClick=(ev => {
                                   ReactEventRe.Mouse.preventDefault(ev);
                                   send(GetRepositories(true));
                               })>(string("Load more"))</a>
                        | None => ReasonReact.null
                        })
                    </div>

                    <div className="column">
                        <h2 className="header">(string("Stargazers"))</h2>
                        (switch(state.selectedRepo) {
                            | Some(repo) =>
                                <Stargazers
                                    url=repo.stargazerUrl
                                    fetchStargazers=client.fetchStargazers
                                    />
                            | None => null
                        })
                    </div>

                </div>
            </div>
        </div>
    }

}